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
#define SAMPLING_NUM_SAMPLES_READ_AT_ONCE 24U // NOLINT(modernize-macro-to-enum)

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

  void (*const doEnableSensorImpl)();  ///< Context: main()
  void (*const doDisableSensorImpl)(); ///< Context: main()
  void (*const doFetchSensorAccelerationImpl)(
      struct Sampling_Acceleration *); ///< Context: main()
  void (*const doWaitDelay5usImpl)(
      struct Sampling_Handle *); ///< Context: main()
  int (*const doForwardAccelerationBufferImpl)(
      const struct Sampling_Acceleration *, uint16_t,
      uint16_t); ///< Context: main()

  void (*const onSamplingStartedCb)();   ///< Context: main()
  void (*const onSamplingStoppedCb)();   ///< Context: main()
  void (*const onSamplingAbortedCb)();   ///< Context: main()
  void (*const onSamplingFinishedCb)();  ///< Context: main()
  void (*const onFifoOverflowCb)();      ///< Context: main()
  void (*const onBufferOverflowCb)();    ///< Context: main()
  void (*const onTransmissionErrorCb)(); ///< Context: main()
};
