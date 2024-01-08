/**
 * \file device_impl.h
 *
 * API of device implementation.
 */

#pragma once
#include <inttypes.h>

struct Sampling_Handle;

#define DEVICEIMPL_DECLARE_HANDLE(HANDLE_NAME, SENSOR_HANDLE_NAME,             \
                                  SAMPLING_HANDLE_NAME)                        \
  struct Controller_Handle HANDLE_NAME = {                                     \
      .deviceVersionMajor = VERSION_MAJOR,                                     \
      .deviceVersionMinor = VERSION_MINOR,                                     \
      .deviceVersionPatch = VERSION_PATCH,                                     \
      .sensorHandle = &SENSOR_HANDLE_NAME,                                     \
      .samplingHandle = &SAMPLING_HANDLE_NAME,                                 \
      .deviceCheckReboot = DeviceImpl_device_checkReboot,                      \
      .deviceRequestReboot = DeviceImpl_device_requestAsyncReboot,             \
      .samplingStart = DeviceImpl_sampling_start,                              \
      .samplingStop = DeviceImpl_sampling_stop,                                \
      .samplingFetchForward = DeviceImpl_sampling_fetchForward,                \
      .samplingSetFifoWatermark = DeviceImpl_sampling_setFifoWatermark,        \
      .samplingClearFifoWatermark = DeviceImpl_sampling_clearFifoWatermark,    \
      .samplingSetFifoOverflow = DeviceImpl_sampling_setFifoOverflow,          \
      .samplingOn5usTimerExpired = DeviceImpl_sampling_on5usTimerExpired}

void DeviceImpl_device_checkReboot();
void DeviceImpl_device_requestAsyncReboot();

void DeviceImpl_sampling_start(struct Sampling_Handle *handle,
                               uint16_t maxSamples);
void DeviceImpl_sampling_stop(struct Sampling_Handle *handle);
int DeviceImpl_sampling_fetchForward(struct Sampling_Handle *handle);
void DeviceImpl_sampling_setFifoWatermark(struct Sampling_Handle *handle);
void DeviceImpl_sampling_clearFifoWatermark(struct Sampling_Handle *handle);
void DeviceImpl_sampling_setFifoOverflow(struct Sampling_Handle *handle);
void DeviceImpl_sampling_on5usTimerExpired(struct Sampling_Handle *handle);
