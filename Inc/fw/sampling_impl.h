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

#define SAMPLING_DECLARE_HANDLE(HANDLE_NAME)                                   \
  struct Sampling_Handle HANDLE_NAME = {                                       \
      .state = {.maxSamples = 0,                                               \
                .doStart = false,                                              \
                .doStop = false,                                               \
                .isStarted = false,                                            \
                .waitFor5usTimer = false,                                      \
                .rxBuffer = {{.x = 0, .y = 0, .z = 0}},                        \
                .isFifoOverflowSet = false,                                    \
                .isFifoWatermarkSet = false,                                   \
                .transactionsCount = 0},                                       \
      .delay5us = SamplingImpl_delay5us,                                       \
      .onSamplingStarted = SamplingImpl_onSamplingStarted,                     \
      .onSamplingStopped = SamplingImpl_onSamplingStopped,                     \
      .onSamplingAborted = SamplingImpl_onSamplingAborted,                     \
      .onSamplingFinished = SamplingImpl_onSamplingFinished,                   \
      .onPostAccelerationBuffer = SamplingImpl_onPostAccelerationBuffer,       \
                                                                               \
      .onFifoOverflow = SamplingImpl_onFifoOverflow,                           \
      .onSensorEnable = SamplingImpl_onSensorEnable,                           \
      .onSensorDisable = SamplingImpl_onSensorDisable,                         \
      .onFetchSensorAcceleration = SamplingImpl_onFetchSensorAcceleration}

void SamplingImpl_delay5us(struct Sampling_Handle *handle);
void SamplingImpl_onSamplingStarted();
void SamplingImpl_onSamplingStopped();
void SamplingImpl_onSamplingAborted();
void SamplingImpl_onSamplingFinished();
void SamplingImpl_onPostAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex);

void SamplingImpl_onFifoOverflow();
void SamplingImpl_onSensorEnable();
void SamplingImpl_onSensorDisable();
void SamplingImpl_onFetchSensorAcceleration(
    struct Sampling_Acceleration *sample);
