/**
 * \file sampling.h
 *
 * Implementation of sampling module.
 */

#include "sampling.h"
#include "sampling_types.h"
#include <errno.h>

static bool isNSamplesReadEnabled(struct Sampling_Handle *handle) {
  return handle->state.maxSamples > 0;
}

static bool checkStartRequest(struct Sampling_Handle *handle) {
  if (!handle->state.doStart) {
    return false;
  }
  handle->state.doStart = false;

  if (handle->state.isStarted) {
    Sampling_stop(handle);
    return false;
  }
  handle->onSamplingStartedCb();

  handle->state.isFifoOverflowSet = false;
  handle->state.transactionsCount = 0;
  handle->state.isStarted = true;

  handle->doEnableSensorImpl();

  return true;
}

static bool checkStopRequest(struct Sampling_Handle *handle) {
  if (!handle->state.doStop) {
    return false;
  }
  handle->state.doStop = false;

  if (!handle->state.isStarted) {
    return false;
  }

  if (handle->state.transactionsCount < handle->state.maxSamples) {
    handle->onSamplingAbortedCb();
  }
  handle->onSamplingStoppedCb();

  handle->doDisableSensorImpl();

  // clear watermark interrupt (fetch complete fifo)
  for (uint8_t idx = 0; idx < ADXL345_FIFO_ENTRIES; idx++) {
    handle->doFetchSensorAccelerationImpl(NULL);
  }

  handle->state.isStarted = false;

  return true;
}

void Sampling_start(struct Sampling_Handle *handle, uint16_t maxSamples) {
  handle->state.maxSamples = maxSamples;
  handle->state.doStart = true;
}

void Sampling_stop(struct Sampling_Handle *handle) {
  handle->state.doStop = true;
}

static bool transmitPending(struct Sampling_Handle *handle) {
  // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
  const int ret = {handle->doForwardAccelerationBufferImpl(NULL, 0, 0)};

  if (ret == -EAGAIN) {
    return true;
  }

  if (ret == -ENOMEM) {
    handle->onBufferOverflowCb();
    Sampling_stop(handle);
  }

  if (ret == -EIO) {
    handle->onTransmissionErrorCb();
    Sampling_stop(handle);
  }

  return false;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int Sampling_fetchForward(struct Sampling_Handle *handle) {
  int retState = {0};
  int retTx = {0};

  checkStartRequest(handle);

  if (handle->state.isFifoWatermarkSet && handle->state.isStarted) {
    uint8_t rxCount = 0;

    // fetch samples
    while (rxCount < SAMPLING_NUM_SAMPLES_READ_AT_ONCE) {

      if (checkStopRequest(handle)) {
        break;
      }

      if (!handle->state.isStarted) {
        retState = -ECANCELED;
        break;
      }

      if (handle->state.isFifoOverflowSet) {
        retState = -EOVERFLOW;
        break;
      }

      if (isNSamplesReadEnabled(handle) &&
          (handle->state.transactionsCount + rxCount) >=
              handle->state.maxSamples) {
        retState = ENODATA;
        break;
      }

      // UM ADXL345 Rev.G p21 - RETRIEVING DATA FROM FIFO:
      // To ensure that the FIFO has completely popped (that is, that new
      // data has completely moved into the DATAX, DATAY, and DATAZ
      // registers), there must be at least 5 µs between the end of reading
      // the data registers and the start of a new read of the FIFO or a read
      // of the FIFO_STATUS register (Address 0x39). The end of reading
      // a data register is signified by the transition from Register 0x37 to
      // Register 0x38 or by the CS pin going high.
      handle->doWaitDelay5usImpl(handle);
      handle->doFetchSensorAccelerationImpl(&handle->state.rxBuffer[rxCount]);
      rxCount++;
    }

    // forward chunk of samples
    if ((0 < rxCount) && handle->state.isStarted) {
      retTx = handle->doForwardAccelerationBufferImpl(
          handle->state.rxBuffer, rxCount, handle->state.transactionsCount);
      handle->state.transactionsCount += rxCount;
    }
  }

  if (-ENOMEM == retTx) {
    handle->onBufferOverflowCb();
    Sampling_stop(handle);
  }

  if (handle->state.isStarted) {
    transmitPending(handle);
  }

  if (-EOVERFLOW == retState) {
    handle->onFifoOverflowCb();
    Sampling_stop(handle);
  }

  if (-ECANCELED == retState) {
    handle->onSamplingAbortedCb();
    Sampling_stop(handle);
  }

  if (ENODATA == retState) {
    while (transmitPending(handle)) {
    }
    handle->onSamplingFinishedCb();
    Sampling_stop(handle);
  }

  checkStopRequest(handle);
  return retTx ? retState == 0 : retState;
}

void Sampling_setFifoWatermark(struct Sampling_Handle *handle) {
  handle->state.isFifoWatermarkSet = true;
}

void Sampling_clearFifoWatermark(struct Sampling_Handle *handle) {
  handle->state.isFifoWatermarkSet = false;
}

void Sampling_setFifoOverflow(struct Sampling_Handle *handle) {
  handle->state.isFifoOverflowSet = true;
}

void Sampling_on5usTimerExpired(struct Sampling_Handle *handle) {
  handle->state.waitFor5usTimer = false;
}
