/**
 * \file device_impl.h
 *
 * API of device implementation.
 */

#pragma once
#include <inttypes.h>

struct Controller_Handle;

#define DEVICEIMPL_DECLARE_HANDLE(HANDLE_NAME)             \
  struct Controller_Handle HANDLE_NAME = {                                     \
      .deviceVersionMajor = VERSION_MAJOR,                                    \
      .deviceVersionMinor = VERSION_MINOR,                                    \
      .deviceVersionPatch = VERSION_PATCH,                                    \
      .deviceCheckReboot = DeviceImpl_device_checkReboot,                     \
      .deviceRequestReboot = DeviceImpl_device_requestAsyncReboot,            \
      .samplingStart = DeviceImpl_sampling_start,                             \
      .samplingStop = DeviceImpl_sampling_stop,                               \
      .samplingFetchForward = DeviceImpl_sampling_fetchForward,               \
      .samplingSetFifoWatermark = DeviceImpl_sampling_setFifoWatermark,       \
      .samplingClearFifoWatermark = DeviceImpl_sampling_clearFifoWatermark,   \
      .samplingSetFifoOverflow = DeviceImpl_sampling_setFifoOverflow,         \
      .samplingOn5usTimerExpired = DeviceImpl_sampling_on5usTimerExpired,     \
  }

void DeviceImpl_device_checkReboot();
void DeviceImpl_device_requestAsyncReboot();

void DeviceImpl_sampling_start(uint16_t maxSamples);
void DeviceImpl_sampling_stop();
int DeviceImpl_sampling_fetchForward();
void DeviceImpl_sampling_setFifoWatermark();
void DeviceImpl_sampling_clearFifoWatermark();
void DeviceImpl_sampling_setFifoOverflow();
void DeviceImpl_sampling_on5usTimerExpired();
