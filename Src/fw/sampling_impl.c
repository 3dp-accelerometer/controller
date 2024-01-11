#include "fw/sampling_impl.h"
#include "tim.h"
#include <controller.h>
#include <sampling_types.h>

extern struct Controller_Handle controllerHandle;

void SamplingImpl_doWaitDelay5us(struct Sampling_Handle *handle) {
  handle->state.waitFor5usTimer = true;
  TIM3->CNT = 0;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim3);

  while (handle->state.waitFor5usTimer) {
  }

  HAL_TIM_Base_Stop_IT(&htim3);
}
