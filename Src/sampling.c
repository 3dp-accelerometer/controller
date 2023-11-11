#include "sampling.h"
#include "adxl345.h"
#include "usb_transport.h"
#include <assert.h>
#include <errno.h>

#define NUM_SAMPLES_READ_AT_ONCE ADXL345_WATERMARK_LEVEL

static_assert(NUM_SAMPLES_READ_AT_ONCE > 0,
              "minimum required watermark level: 1");

struct SamplingState {
  volatile uint16_t maxSamples;
  volatile bool doStart;
  volatile bool doStop;
  volatile bool isStarted;
  struct Adxl345_Acceleration rxBuffer[ADXL345_WATERMARK_LEVEL];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
  int transactionsCount;
};

static struct SamplingState samplingState = {
    .maxSamples = 0,
    .doStart = false,
    .doStop = false,
    .isStarted = false,
    .rxBuffer = {{.x = 0, .y = 0, .z = 0}},
    .isFifoOverflowSet = false,
    .isFifoWatermarkSet = false,
    .transactionsCount = 0,
};

static bool isNSamplesReadEnabled() { return samplingState.maxSamples > 0; }

static void checkStartRequest() {
  if (!samplingState.doStart) {
    return;
  }
  samplingState.doStart = false;

  if (samplingState.isStarted) {
    return;
  }

  TransportTxSamplingStarted(samplingState.maxSamples);

  samplingState.isFifoOverflowSet = false;
  samplingState.transactionsCount = 0;
  samplingState.isStarted = true;

  Adxl345_setPowerCtlMeasure();
}

static void checkStopRequest() {
  if (!samplingState.doStop) {
    return;
  }
  samplingState.doStop = false;

  if (!samplingState.isStarted) {
    return;
  }

  if (samplingState.transactionsCount < samplingState.maxSamples) {
    TransportTxSamplingAborted();
  }
  TransportTxSamplingStopped();

  Adxl345_setPowerCtlStandby();

  // clear watermark interrupt (clear whole fifo)
  struct Adxl345_Acceleration devNull;
  for (uint8_t idx = 0; idx < ADXL345_FIFO_ENTRIES; idx++) {
    Adxl345_getAcceleration(&devNull);
  }

  samplingState.isStarted = false;
}

void sampling_start(uint16_t maxSamples) {
  samplingState.maxSamples = maxSamples;
  samplingState.doStart = true;
}

void sampling_stop() { samplingState.doStop = true; }

int sampling_fetchForward() {
  int ret = {0};

  checkStartRequest();

  if (samplingState.isFifoWatermarkSet && samplingState.isStarted) {
    uint8_t rxCount = 0;

    while (rxCount < NUM_SAMPLES_READ_AT_ONCE) {

      checkStopRequest();

      if (!samplingState.isStarted) {
        ret = -ECANCELED;
        break;
      }

      if (samplingState.isFifoOverflowSet) {
        ret = -EOVERFLOW;
        break;
      }

      if (isNSamplesReadEnabled() && (samplingState.transactionsCount +
                                      rxCount) >= samplingState.maxSamples) {
        ret = ENODATA;
        break;
      }

      Adxl345_getAcceleration(&samplingState.rxBuffer[rxCount]);
      rxCount++;
    }

    if (0 < rxCount) {
      TransportTxAccelerationBuffer(samplingState.rxBuffer, rxCount,
                                    samplingState.transactionsCount);
      samplingState.transactionsCount += rxCount;
    }
  }

  if (-EOVERFLOW == ret) {
    TransportTxFifoOverflow();
    sampling_stop();
  }

  if (-ECANCELED == ret) {
    TransportTxSamplingAborted();
    sampling_stop();
  }

  if (ENODATA == ret) {
    TransportTxSamplingFinished();
    sampling_stop();
  }

  checkStopRequest();
  return ret;
}

void sampling_setFifoWatermark() { samplingState.isFifoWatermarkSet = true; }

void sampling_clearFifoWatermark() { samplingState.isFifoWatermarkSet = false; }

void sampling_setFifoOverflow() { samplingState.isFifoOverflowSet = true; }
