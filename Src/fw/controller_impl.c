/**
 * \file device_impl.c
 *
 * Implements device API.
 */

#include "fw/controller_impl.h"
#include "fw/version.h"
#include <controller.h>
#include <errno.h>
#include <sampling.h>
#include <stm32f4xx_hal.h>

/**
 * Flag indicating a device reboot was requested.
 *
 * \see ControllerImpl_device_requestAsyncReboot()
 */
static bool Controller_rebootRequested = false;

void ControllerImpl_device_checkReboot() {
  if (Controller_rebootRequested) {
    NVIC_SystemReset();
  }
}

void ControllerImpl_device_requestAsyncReboot() {
  Controller_rebootRequested = true;
}

void ControllerImpl_sampling_start(struct Sampling_Handle *handle,
                                   uint16_t maxSamples) {
  Sampling_start(handle, maxSamples);
}

void ControllerImpl_sampling_stop(struct Sampling_Handle *handle) {
  Sampling_stop(handle);
}

int ControllerImpl_sampling_fetchForward(struct Sampling_Handle *handle) {
  return Sampling_fetchForward(handle);
}

void ControllerImpl_sampling_setFifoWatermark(struct Sampling_Handle *handle) {
  Sampling_setFifoWatermark(handle);
}

void ControllerImpl_sampling_clearFifoWatermark(
    struct Sampling_Handle *handle) {
  Sampling_clearFifoWatermark(handle);
}

void ControllerImpl_sampling_setFifoOverflow(struct Sampling_Handle *handle) {
  Sampling_setFifoOverflow(handle);
}

void ControllerImpl_sampling_on5usTimerExpired(struct Sampling_Handle *handle) {
  Sampling_on5usTimerExpired(handle);
}
