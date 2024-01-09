/**
 * \file sampling_impl.h
 *
 * API of sampling implementation.
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

struct Sampling_Handle;

#define SAMPLING_DECLARE_HANDLE(HANDLE_NAME)                                   \
  struct Sampling_Handle HANDLE_NAME = {                                       \
      .maxSamples = 0,                                                         \
      .doStart = false,                                                        \
      .doStop = false,                                                         \
      .isStarted = false,                                                      \
      .waitFor5usTimer = false,                                                \
      .rxBuffer = {{.x = 0, .y = 0, .z = 0}},                                  \
      .isFifoOverflowSet = false,                                              \
      .isFifoWatermarkSet = false,                                             \
      .transactionsCount = 0,                                                  \
      .delay5us = SamplingImpl_delay5us};

void SamplingImpl_delay5us(struct Sampling_Handle *handle);
