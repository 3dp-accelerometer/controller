#include "sampling.h"
#include "adxl345.h"
#include "usbd_cdc_if.h"

struct SamplingState {
  volatile uint16_t maxSamples;
  volatile bool isEnabled;
  char txBuffer[32];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
};

static struct SamplingState samplingState = {
    .maxSamples = 0,
    .isEnabled = false,
    .txBuffer = {0},
    .isFifoOverflowSet = false,
    .isFifoWatermarkSet = false,
};

void sampling_start() {
  samplingState.maxSamples = 0;
  samplingState.isEnabled = true;
}

void sampling_startN(uint16_t maxSamples) {
  samplingState.maxSamples = maxSamples;
  samplingState.isEnabled = true;
}

void sampling_stop() { samplingState.isEnabled = true; }

int sampling_fetchForward() {
  int transactions = 0;

  if (!samplingState.isEnabled) {
    return 0;
  }

  while (samplingState.isFifoWatermarkSet) {

    if (sampling_hasFifoOverflow()) {
      samplingState.isEnabled = false;
      break;
    }

    // enough samples transmitted
    if (0 != samplingState.maxSamples &&
        transactions >= samplingState.maxSamples) {
      samplingState.isEnabled = false;
      break;
    }

    struct Adxl345_Acceleration acc;
    Adxl345_getAcceleration(&acc);
    // todo: CDC_Transmit_FS((uint8_t *)&acc, sizeof(struct
    // Adxl345_Acceleration));
    sprintf(samplingState.txBuffer, "%d %d %d\r\n", acc.x, acc.y, acc.z);
    CDC_Transmit_FS(
        (uint8_t *)samplingState.txBuffer,
        strnlen(samplingState.txBuffer, sizeof(samplingState.txBuffer)));

    transactions++;
  }

  return transactions;
}

void sampling_setFifoWatermark() { samplingState.isFifoWatermarkSet = true; }

void sampling_clearFifoWatermark() { samplingState.isFifoWatermarkSet = false; }

void sampling_setFifoOverflow() { samplingState.isFifoOverflowSet = true; }

void sampling_clearFifoOverflow() { samplingState.isFifoOverflowSet = false; }

bool sampling_hasFifoOverflow() { return samplingState.isFifoOverflowSet; }
