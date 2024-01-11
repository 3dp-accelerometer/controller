/**
 * \file sampling_impl.h
 *
 * API of sampling implementation.
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

struct Sampling_Acceleration;
struct Sampling_Handle;

#define SAMPLING_DECLARE_INITIALIZER                                           \
  {                                                                            \
    .state = {.maxSamples = 0,                                                 \
              .doStart = false,                                                \
              .doStop = false,                                                 \
              .isStarted = false,                                              \
              .waitFor5usTimer = false,                                        \
              .rxBuffer = {{.x = 0, .y = 0, .z = 0}},                          \
              .isFifoOverflowSet = false,                                      \
              .isFifoWatermarkSet = false,                                     \
              .transactionsCount = 0},                                         \
                                                                               \
    .doEnableSensorImpl = ControllerImpl_sampling_doEnableSensorImpl,          \
    .doDisableSensorImpl = ControllerImpl_sampling_doDisableSensorImpl,        \
    .doFetchSensorAccelerationImpl =                                           \
        ControllerImpl_sampling_doFetchSensorAccelerationImpl,                 \
    .doWaitDelay5usImpl = SamplingImpl_doWaitDelay5usImpl,                     \
    .doForwardAccelerationBufferImpl =                                         \
        ControllerImpl_sampling_doForwardAccelerationBufferImpl,               \
                                                                               \
    .onSamplingStartedCb = ControllerImpl_sampling_onSamplingStartedCb,        \
    .onSamplingStoppedCb = ControllerImpl_sampling_onSamplingStoppedCb,        \
    .onSamplingAbortedCb = ControllerImpl_sampling_onSamplingAbortedCb,        \
    .onSamplingFinishedCb = ControllerImpl_sampling_onSamplingFinishedCb,      \
    .onFifoOverflowCb = ControllerImpl_sampling_onFifoOverflowCb,              \
  }

void SamplingImpl_doWaitDelay5usImpl(struct Sampling_Handle *handle);
