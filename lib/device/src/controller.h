/**
 * \file device_impl.h
 *
 * API of device implementation.
 */

#pragma once
#include <inttypes.h>
#include <stdbool.h>

struct Adxl345_Handle;

struct Controller_Handle {
  uint8_t deviceVersionMajor;
  uint8_t deviceVersionMinor;
  uint8_t deviceVersionPatch;
  void (*deviceCheckReboot)();
  void (*deviceRequestReboot)();

  void (*samplingStart)(uint16_t);
  void (*samplingStop)();
  int (*samplingFetchForward)();
  void (*samplingSetFifoWatermark)();
  void (*samplingClearFifoWatermark)();
  void (*samplingSetFifoOverflow)();
  void (*samplingOn5usTimerExpired)();
};
