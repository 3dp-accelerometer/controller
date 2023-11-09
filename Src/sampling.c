#include "sampling.h"
#include "adxl345.h"
#include "usb_transport.h"
#include "usbd_cdc_if.h"
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
  struct Adxl345_Acceleration rxBuffer[NUM_SAMPLES_READ_AT_ONCE];
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

static void usbSendSamplingStarted() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_SamplingStarted};
  USBD_StatusTypeDef ret = CDC_Transmit_FS(
      (uint8_t *)&tx,
      sizeof(struct TransportHeader) + sizeof(tx.asTxFrame.asSamplingStarted));

  while (USBD_BUSY == ret) {
    ret = CDC_Transmit_FS((uint8_t *)&tx,
                          sizeof(struct TransportHeader) +
                              sizeof(tx.asTxFrame.asSamplingStarted));
  }
}

static void usbSendSamplingFinished() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_SamplingFinished};
  USBD_StatusTypeDef ret = CDC_Transmit_FS(
      (uint8_t *)&tx,
      sizeof(struct TransportHeader) + sizeof(tx.asTxFrame.asSamplingFinished));

  while (USBD_BUSY == ret) {
    ret = CDC_Transmit_FS((uint8_t *)&tx,
                          sizeof(struct TransportHeader) +
                              sizeof(tx.asTxFrame.asSamplingFinished));
  }
}

static void usbSendSamplingStopped() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_SamplingStopped};

  USBD_StatusTypeDef ret = CDC_Transmit_FS(
      (uint8_t *)&tx,
      sizeof(struct TransportHeader) + sizeof(tx.asTxFrame.asSamplingStopped));

  while (USBD_BUSY == ret) {
    ret = CDC_Transmit_FS((uint8_t *)&tx,
                          sizeof(struct TransportHeader) +
                              sizeof(tx.asTxFrame.asSamplingStopped));
  }
}

static void usbSendSamplingAborted() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_SamplingAborted};
  USBD_StatusTypeDef ret = CDC_Transmit_FS(
      (uint8_t *)&tx,
      sizeof(struct TransportHeader) + sizeof(tx.asTxFrame.asSamplingAborted));

  while (USBD_BUSY == ret) {
    ret = CDC_Transmit_FS((uint8_t *)&tx,
                          sizeof(struct TransportHeader) +
                              sizeof(tx.asTxFrame.asSamplingAborted));
  }
}

static void usbSendFifoOverflow() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_FifoOverflow};
  USBD_StatusTypeDef ret =
      CDC_Transmit_FS((uint8_t *)&tx, sizeof(struct TransportHeader) +
                                          sizeof(tx.asTxFrame.asFifoOverflow));

  while (USBD_BUSY == ret) {
    ret = CDC_Transmit_FS((uint8_t *)&tx,
                          sizeof(struct TransportHeader) +
                              sizeof(tx.asTxFrame.asFifoOverflow));
  }
}

static void usbSendAccelerationBuffer(struct Adxl345_Acceleration *buffer,
                                      uint8_t count) {
  HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  struct TransportFrame acc[NUM_SAMPLES_READ_AT_ONCE] = {};

  // todo
  assert(sizeof(struct TransportFrame) ==
         (sizeof(struct TransportTx_Acceleration) +
          sizeof(struct TransportHeader)));

  for (uint8_t idx = 0; idx < count; idx++) {
    acc[idx].asTxFrame.asAcceleration.x = buffer[idx].x;
    acc[idx].asTxFrame.asAcceleration.y = buffer[idx].y;
    acc[idx].asTxFrame.asAcceleration.z = buffer[idx].z;
    acc[idx].header.id = TransportHeader_Id_Acceleration;
  }

  USBD_StatusTypeDef ret =
      CDC_Transmit_FS((uint8_t *)acc, count * sizeof(struct TransportFrame));
  while (USBD_BUSY == ret) {
    ret =
        CDC_Transmit_FS((uint8_t *)acc, count * sizeof(struct TransportFrame));
  }
  HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
}

static void checkStartRequest() {
  if (!samplingState.doStart) {
    return;
  }
  samplingState.doStart = false;

  if (samplingState.isStarted) {
    return;
  }

  usbSendSamplingStarted();

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
    usbSendSamplingAborted();
  }
  usbSendSamplingStopped();

  Adxl345_setPowerCtlStandby();

  // clear watermark interrupt
  struct Adxl345_Acceleration devNull;
  for (uint8_t idx = 0; idx < ADXL345_FIFO_SIZE; idx++) {
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
      usbSendAccelerationBuffer(samplingState.rxBuffer, rxCount);
      samplingState.transactionsCount += rxCount;
    }
  }

  if (-EOVERFLOW == ret) {
    usbSendFifoOverflow();
    sampling_stop();
  }

  if (-ECANCELED == ret) {
    usbSendSamplingAborted();
    sampling_stop();
  }

  if (ENODATA == ret) {
    usbSendSamplingFinished();
    sampling_stop();
  }

  checkStopRequest();
  return ret;
}

void sampling_setFifoWatermark() { samplingState.isFifoWatermarkSet = true; }

void sampling_clearFifoWatermark() { samplingState.isFifoWatermarkSet = false; }

void sampling_setFifoOverflow() { samplingState.isFifoOverflowSet = true; }
