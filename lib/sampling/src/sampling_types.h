/**
 * \file sampling_types.h
 *
 * Sampling handle and constants.
 */

#pragma once

#include <adxl345.h>
#include <adxl345_transport_types.h>
#include <inttypes.h>
#include <stdbool.h>

/**
 * Configures how many samples can be maximally read at once from the sensor.
 *
 * Must be less than or equal watermark level to not read beyond buffered FiFo.
 */
#define NUM_SAMPLES_READ_AT_ONCE ADXL345_WATERMARK_LEVEL

#define MYSTRINGIZE0(A) #A
#define MYSTRINGIZE(A) MYSTRINGIZE0(A)

static_assert(
    NUM_SAMPLES_READ_AT_ONCE <= ADXL345_WATERMARK_LEVEL,
    "maximum allowed read-at-once: " MYSTRINGIZE(ADXL345_WATERMARK_LEVEL));

static_assert(ADXL345_WATERMARK_LEVEL > 0,
              "minimum required watermark level: 1");
#undef MYSTRINGIZE
#undef MYSTRINGIZE0

/**
 * Internal module state and device specific implementation.
 */
struct Sampling_Handle {
  volatile uint16_t maxSamples;
  volatile bool doStart;
  volatile bool doStop;
  volatile bool isStarted;
  volatile bool waitFor5usTimer;
  struct Adxl345Transport_Acceleration rxBuffer[NUM_SAMPLES_READ_AT_ONCE];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
  int transactionsCount;

  void (*delay5us)(struct Sampling_Handle *);
};
