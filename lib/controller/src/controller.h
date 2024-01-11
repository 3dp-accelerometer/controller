/**
 * \file controller.h
 *
 * API of controller.
 */

#pragma once
#include <inttypes.h>
#include <stdbool.h>

struct Adxl345_Handle;
struct HostTransport_Handle;
struct Sampling_Handle;
struct Sampling_Acceleration;

enum TransportRx_SetOutputDataRate_Rate;
enum TransportRx_SetScale_Scale;
enum TransportRx_SetRange_Range;

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
  uint8_t swVersionMajor;
  uint8_t swVersionMinor;
  uint8_t swVersionPatch;

  struct Adxl345_Handle *sensorHandle;
  struct Sampling_Handle *samplingHandle;
  struct HostTransport_Handle *hostTransportHandle;

  void (*init)();
  void (*loop)();

  void (*sensorInit)();

  void (*controllerCheckReboot)();
  void (*controllerRequestReboot)();

  void (*hostOnBytesReceived)(uint8_t *, uint16_t);
  int (*hostOnRequestGetFirmwareVersion)();
  int (*hostOnRequestGetOutputDataRate)();
  int (*hostOnRequestSetOutputDatatRate)(
      enum TransportRx_SetOutputDataRate_Rate);
  int (*hostOnRequestGetRange)();
  int (*hostOnRequestSetRange)(enum TransportRx_SetRange_Range);
  int (*hostOnRequestGetScale)();
  int (*hostOnRequestSetScale)(enum TransportRx_SetScale_Scale);
  int (*hostOnRequestGetDeviceSetup)();
  int (*hostOnRequestSamplingStart)(uint16_t);
  int (*hostOnRequestSamplingStop)();

  void (*samplingStart)(uint16_t);
  void (*samplingStop)();
  int (*samplingFetchForward)();
  void (*samplingSetFifoWatermark)();
  void (*samplingClearFifoWatermark)();
  void (*samplingSetFifoOverflow)();
  void (*samplingOn5usTimerExpired)();
  void (*samplingOnStarted)();
  void (*samplingOnStopped)();
  void (*samplingOnAborted)();
  void (*samplingOnFinished)();
  void (*samplingOnPostAccelerationBuffer)(const struct Sampling_Acceleration *,
                                           uint16_t, uint16_t);
  void (*samplingOnFifoOverflow)();
  void (*samplingOnSensorEnable)();
  void (*samplingOnSensorDisable)();
  void (*samplingOnFetchSensorAcceleration)(struct Sampling_Acceleration *);
};
