#include "fw/sampling.h"
#include "fw/adxl345.h"
#include "fw/adxl345_transport_types.h"
#include "fw/usbd_cdc_transport.h"
#include "tim.h"
#include <assert.h>
#include <errno.h>

/**
 * Configures how many samples can be maximally read at once from the sensor.
 *
 * Must be less than or equal watermark level to not read beyond buffered FiFo.
 */
#define NUM_SAMPLES_READ_AT_ONCE ADXL345_WATERMARK_LEVEL

#define STRINGIZE0(A) #A
#define STRINGIZE(A) STRINGIZE0(A)

static_assert(
    NUM_SAMPLES_READ_AT_ONCE <= ADXL345_WATERMARK_LEVEL,
    "maximum allowed read-at-once: " STRINGIZE(ADXL345_WATERMARK_LEVEL));

static_assert(ADXL345_WATERMARK_LEVEL > 0,
              "minimum required watermark level: 1");
#undef STRINGIZE
#undef STRINGIZE0

/**
 * Internal module state.
 */
struct SamplingState {
  volatile uint16_t maxSamples;
  volatile bool doStart;
  volatile bool doStop;
  volatile bool isStarted;
  volatile bool waitFor5usTimer;
  struct Adxl345TP_Acceleration rxBuffer[NUM_SAMPLES_READ_AT_ONCE];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
  int transactionsCount;
};

/**
 * Internal module state.
 */
static struct SamplingState samplingState = {
    .maxSamples = 0,
    .doStart = false,
    .doStop = false,
    .isStarted = false,
    .waitFor5usTimer = false,
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

  TransportTx_FirmwareVersion();
  TransportTx_SamplingStarted(samplingState.maxSamples);

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
    TransportTx_SamplingAborted();
  }
  TransportTx_SamplingStopped();

  Adxl345_setPowerCtlStandby();

  // clear watermark interrupt (clear whole fifo)
  struct Adxl345TP_Acceleration devNull;
  for (uint8_t idx = 0; idx < ADXL345_FIFO_ENTRIES; idx++) {
    Adxl345_getAcceleration(&devNull);
  }

  samplingState.isStarted = false;
}

static void delay5us() {
  samplingState.waitFor5usTimer = true;
  TIM3->CNT = 0;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim3);

  while (samplingState.waitFor5usTimer)
    ;

  HAL_TIM_Base_Stop_IT(&htim3);
}

void Sampling_start(uint16_t maxSamples) {
  samplingState.maxSamples = maxSamples;
  samplingState.doStart = true;
}

void Sampling_stop() { samplingState.doStop = true; }

int Sampling_fetchForward() {
  int ret = {0};

  checkStartRequest();

  if (samplingState.isFifoWatermarkSet && samplingState.isStarted) {
    uint8_t rxCount = 0;

    // fetch samples
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

      // UM ADXL345 Rev.G p21 - RETRIEVING DATA FROM FIFO:
      // To ensure that the FIFO has completely popped (that is, that new
      // data has completely moved into the DATAX, DATAY, and DATAZ
      // registers), there must be at least 5 µs between the end of reading
      // the data registers and the start of a new read of the FIFO or a read
      // of the FIFO_STATUS register (Address 0x39). The end of reading
      // a data register is signified by the transition from Register 0x37 to
      // Register 0x38 or by the CS pin going high.
      delay5us();
      Adxl345_getAcceleration(&samplingState.rxBuffer[rxCount]);
      rxCount++;
    }

    // forward samples
    if (0 < rxCount) {
      TransportTx_AccelerationBuffer(samplingState.rxBuffer, rxCount,
                                     samplingState.transactionsCount);
      samplingState.transactionsCount += rxCount;
    }
  }

  if (-EOVERFLOW == ret) {
    TransportTx_FifoOverflow();
    Sampling_stop();
  }

  if (-ECANCELED == ret) {
    TransportTx_SamplingAborted();
    Sampling_stop();
  }

  if (ENODATA == ret) {
    TransportTx_SamplingFinished();
    Sampling_stop();
  }

  checkStopRequest();
  return ret;
}

void Sampling_setFifoWatermark() { samplingState.isFifoWatermarkSet = true; }

void Sampling_clearFifoWatermark() { samplingState.isFifoWatermarkSet = false; }

void Sampling_setFifoOverflow() { samplingState.isFifoOverflowSet = true; }

void Sampling_on5usTimerExpired() { samplingState.waitFor5usTimer = false; }
