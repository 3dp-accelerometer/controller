/**
 * \file device_impl.c
 *
 * Implements device API.
 */

#include "fw/controller_impl.h"
#include "fw/controller_reboot.h"
#include "fw/sampling.h"
#include "fw/version.h"
#include <controller.h>
#include <errno.h>

void DeviceImpl_device_checkReboot() { ControllerReboot_checkReboot(); }

void DeviceImpl_device_requestAsyncReboot() {
  ControllerReboot_requestAsyncReboot();
}

void DeviceImpl_sampling_start(struct Sampling_Handle *handle,
                               uint16_t maxSamples) {
  Sampling_start(handle, maxSamples);
}

void DeviceImpl_sampling_stop(struct Sampling_Handle *handle) {
  Sampling_stop(handle);
}

int DeviceImpl_sampling_fetchForward(struct Sampling_Handle *handle) {
  return Sampling_fetchForward(handle);
}

void DeviceImpl_sampling_setFifoWatermark(struct Sampling_Handle *handle) {
  Sampling_setFifoWatermark(handle);
}

void DeviceImpl_sampling_clearFifoWatermark(struct Sampling_Handle *handle) {
  Sampling_clearFifoWatermark(handle);
}

void DeviceImpl_sampling_setFifoOverflow(struct Sampling_Handle *handle) {
  Sampling_setFifoOverflow(handle);
}

void DeviceImpl_sampling_on5usTimerExpired(struct Sampling_Handle *handle) {
  Sampling_on5usTimerExpired(handle);
}
