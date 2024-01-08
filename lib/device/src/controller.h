/**
 * \file device_impl.h
 *
 * API of device implementation.
 */

#pragma once
#include <inttypes.h>
#include <stdbool.h>

struct Adxl345_Handle;
struct Sampling_Handle;

/**
 * Handle for several device pointer implementations.
 *
 * This handle will be shared among main() context and several interrupts'
 * context.Hence, all members shall be seen as const and never be modified at
 * runtime.Underlying fields may be changed though. These shall be marked as
 * volatile and properly synchronized.
 *
 * todo: mark underlying fields as volatile
 * todo: introduce locks for synchronization
 * todo: read the docs if parallel interrupts exist on stm32
 */
struct Controller_Handle {
  uint8_t deviceVersionMajor;
  uint8_t deviceVersionMinor;
  uint8_t deviceVersionPatch;

  struct Adxl345_Handle *sensorHandle;
  struct Sampling_Handle *samplingHandle;

  void (*deviceCheckReboot)();
  void (*deviceRequestReboot)();

  void (*samplingStart)(struct Sampling_Handle *handle, uint16_t);
  void (*samplingStop)(struct Sampling_Handle *handle);
  int (*samplingFetchForward)(struct Sampling_Handle *handle);
  void (*samplingSetFifoWatermark)(struct Sampling_Handle *handle);
  void (*samplingClearFifoWatermark)(struct Sampling_Handle *handle);
  void (*samplingSetFifoOverflow)(struct Sampling_Handle *handle);
  void (*samplingOn5usTimerExpired)(struct Sampling_Handle *handle);
};
