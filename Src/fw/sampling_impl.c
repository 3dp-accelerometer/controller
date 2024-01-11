#include "fw/sampling_impl.h"
#include "tim.h"
#include <controller.h>
#include <sampling_types.h>

extern struct Controller_Handle controllerHandle;

void SamplingImpl_delay5us(struct Sampling_Handle *handle) {
  handle->state.waitFor5usTimer = true;
  TIM3->CNT = 0;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Start_IT(&htim3);

  while (handle->state.waitFor5usTimer) {
  }

  HAL_TIM_Base_Stop_IT(&htim3);
}

void SamplingImpl_onSamplingStarted() { controllerHandle.samplingOnStarted(); }

void SamplingImpl_onSamplingStopped() { controllerHandle.samplingOnStopped(); }

void SamplingImpl_onSamplingAborted() { controllerHandle.samplingOnAborted(); }

void SamplingImpl_onSamplingFinished() {
  controllerHandle.samplingOnFinished();
}

void SamplingImpl_onPostAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex) {
  controllerHandle.samplingOnPostAccelerationBuffer(buffer, bufferLen,
                                                    startIndex);
}

void SamplingImpl_onFifoOverflow() {
  controllerHandle.samplingOnFifoOverflow();
}

void SamplingImpl_onSensorEnable() {
  controllerHandle.samplingOnSensorEnable();
}

void SamplingImpl_onSensorDisable() {
  controllerHandle.samplingOnSensorDisable();
}

void SamplingImpl_onFetchSensorAcceleration(
    struct Sampling_Acceleration *sample) {
  controllerHandle.samplingOnFetchSensorAcceleration(sample);
}
