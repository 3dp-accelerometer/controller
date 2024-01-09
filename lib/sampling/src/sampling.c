/**
 * \file sampling.h
 *
 * Implementation of sampling module.
 */

#include "sampling.h"
#include "sampling_types.h"
#include <assert.h>
#include <controller.h>
#include <errno.h>
#include <to_host_transport.h>

extern struct Adxl345_Handle sensorHandle;
extern struct HostTransport_Handle hostHandle;
extern struct Controller_Handle controllerHandle;

static bool isNSamplesReadEnabled(struct Sampling_Handle *handle) {
  return handle->maxSamples > 0;
}

static void checkStartRequest(struct Sampling_Handle *handle) {
  if (!handle->doStart) {
    return;
  }
  handle->doStart = false;

  if (handle->isStarted) {
    return;
  }

  TransportTx_FirmwareVersion(&hostHandle, &controllerHandle);
  TransportTx_SamplingStarted(&hostHandle, handle->maxSamples);

  handle->isFifoOverflowSet = false;
  handle->transactionsCount = 0;
  handle->isStarted = true;

  Adxl345_setPowerCtlMeasure(&sensorHandle);
}

static void checkStopRequest(struct Sampling_Handle *handle) {
  if (!handle->doStop) {
    return;
  }
  handle->doStop = false;

  if (!handle->isStarted) {
    return;
  }

  if (handle->transactionsCount < handle->maxSamples) {
    TransportTx_SamplingAborted(&hostHandle);
  }
  TransportTx_SamplingStopped(&hostHandle, &sensorHandle);

  Adxl345_setPowerCtlStandby(&sensorHandle);

  // clear watermark interrupt (clear whole fifo)
  struct Adxl345Transport_Acceleration devNull;
  for (uint8_t idx = 0; idx < ADXL345_FIFO_ENTRIES; idx++) {
    Adxl345_getAcceleration(&sensorHandle, &devNull);
  }

  handle->isStarted = false;
}

void Sampling_start(struct Sampling_Handle *handle, uint16_t maxSamples) {
  handle->maxSamples = maxSamples;
  handle->doStart = true;
}

void Sampling_stop(struct Sampling_Handle *handle) { handle->doStop = true; }

int Sampling_fetchForward(struct Sampling_Handle *handle) {
  int ret = {0};

  checkStartRequest(handle);

  if (handle->isFifoWatermarkSet && handle->isStarted) {
    uint8_t rxCount = 0;

    // fetch samples
    while (rxCount < NUM_SAMPLES_READ_AT_ONCE) {

      checkStopRequest(handle);

      if (!handle->isStarted) {
        ret = -ECANCELED;
        break;
      }

      if (handle->isFifoOverflowSet) {
        ret = -EOVERFLOW;
        break;
      }

      if (isNSamplesReadEnabled(handle) &&
          (handle->transactionsCount + rxCount) >= handle->maxSamples) {
        ret = ENODATA;
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
      handle->delay5us(handle);
      Adxl345_getAcceleration(&sensorHandle, &handle->rxBuffer[rxCount]);
      rxCount++;
    }

    // forward samples
    if (0 < rxCount) {
      TransportTx_AccelerationBuffer(&hostHandle, handle->rxBuffer, rxCount,
                                     handle->transactionsCount);
      handle->transactionsCount += rxCount;
    }
  }

  if (-EOVERFLOW == ret) {
    TransportTx_FifoOverflow(&hostHandle);
    Sampling_stop(handle);
  }

  if (-ECANCELED == ret) {
    TransportTx_SamplingAborted(&hostHandle);
    Sampling_stop(handle);
  }

  if (ENODATA == ret) {
    TransportTx_SamplingFinished(&hostHandle);
    Sampling_stop(handle);
  }

  checkStopRequest(handle);
  return ret;
}

void Sampling_setFifoWatermark(struct Sampling_Handle *handle) {
  handle->isFifoWatermarkSet = true;
}

void Sampling_clearFifoWatermark(struct Sampling_Handle *handle) {
  handle->isFifoWatermarkSet = false;
}

void Sampling_setFifoOverflow(struct Sampling_Handle *handle) {
  handle->isFifoOverflowSet = true;
}

void Sampling_on5usTimerExpired(struct Sampling_Handle *handle) {
  handle->waitFor5usTimer = false;
}
