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
  volatile uint16_t maxSamples;  ///< Context: main() and interrupts
  bool doStart;                  ///< Context: main()
  bool doStop;                   ///< Context: main()
  bool isStarted;                ///< Context: main()
  volatile bool waitFor5usTimer; ///< Context: main() and interrupts
  struct Sampling_Acceleration
      rxBuffer[SAMPLING_NUM_SAMPLES_READ_AT_ONCE]; ///< Context: main()
  volatile bool isFifoOverflowSet;  ///< Context: main() and interrupts
  volatile bool isFifoWatermarkSet; ///< Context: main() and interrupts
  int transactionsCount;            ///< Context: main()
};

/**
 * Internal module state and device specific implementation.
 */
struct Sampling_Handle {
  struct Sampling_State state;

  void (*const doEnableSensor)();  ///< Context: main()
  void (*const doDisableSensor)(); ///< Context: main()
  void (*const doFetchSensorAcceleration)(
      struct Sampling_Acceleration *);                    ///< Context: main()
  void (*const doWaitDelay5us)(struct Sampling_Handle *); ///< Context: main()
  void (*const doForwardAccelerationBuffer)(
      const struct Sampling_Acceleration *, uint16_t,
      uint16_t); ///< Context: main()

  void (*const onSamplingStarted)();  ///< Context: main()
  void (*const onSamplingStopped)();  ///< Context: main()
  void (*const onSamplingAborted)();  ///< Context: main()
  void (*const onSamplingFinished)(); ///< Context: main()
  void (*const onFifoOverflow)();     ///< Context: main()
};
