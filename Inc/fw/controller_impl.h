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
      .swVersionMajor = VERSION_MAJOR,                                         \
      .swVersionMinor = VERSION_MINOR,                                         \
      .swVersionPatch = VERSION_PATCH,                                         \
      .sensorHandle = &SENSOR_HANDLE_NAME,                                     \
      .samplingHandle = &SAMPLING_HANDLE_NAME,                                 \
      .controllerCheckReboot = ControllerImpl_device_checkReboot,              \
      .controllerRequestReboot = ControllerImpl_device_requestAsyncReboot,     \
      .samplingStart = ControllerImpl_sampling_start,                          \
      .samplingStop = ControllerImpl_sampling_stop,                            \
      .samplingFetchForward = ControllerImpl_sampling_fetchForward,            \
      .samplingSetFifoWatermark = ControllerImpl_sampling_setFifoWatermark,    \
      .samplingClearFifoWatermark =                                            \
          ControllerImpl_sampling_clearFifoWatermark,                          \
      .samplingSetFifoOverflow = ControllerImpl_sampling_setFifoOverflow,      \
      .samplingOn5usTimerExpired = ControllerImpl_sampling_on5usTimerExpired}

void ControllerImpl_device_checkReboot();
void ControllerImpl_device_requestAsyncReboot();

void ControllerImpl_sampling_start(struct Sampling_Handle *handle,
                                   uint16_t maxSamples);
void ControllerImpl_sampling_stop(struct Sampling_Handle *handle);
int ControllerImpl_sampling_fetchForward(struct Sampling_Handle *handle);
void ControllerImpl_sampling_setFifoWatermark(struct Sampling_Handle *handle);
void ControllerImpl_sampling_clearFifoWatermark(struct Sampling_Handle *handle);
void ControllerImpl_sampling_setFifoOverflow(struct Sampling_Handle *handle);
void ControllerImpl_sampling_on5usTimerExpired(struct Sampling_Handle *handle);
