/**
 * \file sampling_types.h
 *
 * Sampling handle and constants.
 */

#pragma once

#include <adxl345.h>
#include <inttypes.h>
#include <stdbool.h>

/**
 * Configures how many samples can be maximally read at once from the sensor.
 *
 * Must be less than or equal watermark level to not read beyond buffered FiFo.
 */
#define SAMPLING_NUM_SAMPLES_READ_AT_ONCE 24

struct Sampling_Acceleration {
  int16_t x;
  int16_t y;
  int16_t z;
} __attribute__((packed));

/**
 * Internal module state.
 */
struct Sampling_State {
  volatile uint16_t maxSamples;
  volatile bool doStart;
  volatile bool doStop;
  volatile bool isStarted;
  volatile bool waitFor5usTimer;
  struct Sampling_Acceleration rxBuffer[SAMPLING_NUM_SAMPLES_READ_AT_ONCE];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
  volatile int transactionsCount;
};

/**
 * Internal module state and device specific implementation.
 */
struct Sampling_Handle {
  struct Sampling_State state;

  void (*doEnableSensor)();
  void (*doDisableSensor)();
  void (*doFetchSensorAcceleration)(struct Sampling_Acceleration *);
  void (*doWaitDelay5us)(struct Sampling_Handle *);
  void (*doForwardAccelerationBuffer)(const struct Sampling_Acceleration *,
                                      uint16_t, uint16_t);

  void (*onSamplingStarted)();
  void (*onSamplingStopped)();
  void (*onSamplingAborted)();
  void (*onSamplingFinished)();
  void (*onFifoOverflow)();
};
