#include <sampling_types.h>
#include "tim.h"

extern struct Adxl345_Handle sensorHandle;
extern struct HostTransport_Handle hostHandle;
extern struct Controller_Handle controllerHandle;

void SamplingImpl_delay5us(struct Sampling_Handle *handle) {
  handle->waitFor5usTimer = true;
  TIM3->CNT = 0;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim3);

  while (handle->waitFor5usTimer)
    ;

  HAL_TIM_Base_Stop_IT(&htim3);
}
