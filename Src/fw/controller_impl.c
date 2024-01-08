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

void DeviceImpl_sampling_start(uint16_t maxSamples) {
  Sampling_start(maxSamples);
}

void DeviceImpl_sampling_stop() { Sampling_stop(); }
int DeviceImpl_sampling_fetchForward() { return Sampling_fetchForward(); }
void DeviceImpl_sampling_setFifoWatermark() { Sampling_setFifoWatermark(); }
void DeviceImpl_sampling_clearFifoWatermark() { Sampling_clearFifoWatermark(); }
void DeviceImpl_sampling_setFifoOverflow() { Sampling_setFifoOverflow(); }
void DeviceImpl_sampling_on5usTimerExpired() { Sampling_on5usTimerExpired(); }
