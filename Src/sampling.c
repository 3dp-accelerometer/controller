#include "sampling.h"
#include "adxl345.h"
#include "main.h"
#include "usbd_cdc_if.h"
#include <stm32f4xx_hal_gpio.h>

#define WATERMARK_GPIO_Port ACC_INT1_GPIO_Port
#define WATERMARK_Pin ACC_INT1_Pin

#define OVF_GPIO_Port ACC_INT2_GPIO_Port
#define OVF_Pin ACC_INT2_Pin

struct SamplingState {
  uint16_t maxSamples;
  bool isEnabled;
  char txBuffer[32];
  bool fifoOverflow;
};

static struct SamplingState samplingState = {.maxSamples = 0,
                                             .isEnabled = false,
                                             .txBuffer = {0},
                                             .fifoOverflow = false};

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

  while (GPIO_PIN_SET == HAL_GPIO_ReadPin(WATERMARK_GPIO_Port, WATERMARK_Pin)) {

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
    // CDC_Transmit_FS((uint8_t *)&acc, sizeof(struct Adxl345_Acceleration));
    sprintf(samplingState.txBuffer, "%d %d %d\r\n", acc.x, acc.y, acc.z);
    CDC_Transmit_FS(
        (uint8_t *)samplingState.txBuffer,
        strnlen(samplingState.txBuffer, sizeof(samplingState.txBuffer)));

    transactions++;
  }

  return transactions;
}

void sampling_setFifoOverflow() { samplingState.fifoOverflow = true; }

void sampling_clearFifoOverflow() { samplingState.fifoOverflow = false; }

bool sampling_hasFifoOverflow() { return samplingState.fifoOverflow; }
