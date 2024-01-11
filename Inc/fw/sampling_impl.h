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
    .doEnableSensor = ControllerImpl_sampling_doEnableSensor,                  \
    .doDisableSensor = ControllerImpl_sampling_doDisableSensor,                \
    .doFetchSensorAcceleration =                                               \
        ControllerImpl_sampling_doFetchSensorAcceleration,                     \
    .doWaitDelay5us = SamplingImpl_doWaitDelay5us,                             \
    .doForwardAccelerationBuffer =                                             \
        ControllerImpl_sampling_doForwardAccelerationBuffer,                   \
                                                                               \
    .onSamplingStarted = ControllerImpl_sampling_onSamplingStarted,            \
    .onSamplingStopped = ControllerImpl_sampling_onSamplingStopped,            \
    .onSamplingAborted = ControllerImpl_sampling_onSamplingAborted,            \
    .onSamplingFinished = ControllerImpl_sampling_onSamplingFinished,          \
    .onFifoOverflow = ControllerImpl_sampling_onFifoOverflow,                  \
  }

void SamplingImpl_doWaitDelay5us(struct Sampling_Handle *handle);
