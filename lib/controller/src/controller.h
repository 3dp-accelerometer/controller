/**
 * \file controller.h
 *
 * API of controller.
 */

#pragma once
#include <inttypes.h>
#include <stdbool.h>

#include <adxl345.h>
#include <host_transport.h>
#include <sampling_types.h>

enum TransportRx_SetOutputDataRate_Rate;
enum TransportRx_SetScale_Scale;
enum TransportRx_SetRange_Range;

struct Controller_Sensor {
  struct Adxl345_Handle handle;

  void (*init)();
};

struct Controller_Sampling {
  struct Sampling_Handle handle; ///< device specific pimpl

  /**
   * Device API for sampling module.
   *
   * @{
   */
  void (*doSetFifoWatermark)();
  void (*doClearFifoWatermark)();
  void (*doSetFifoOverflow)();
  void (*doSet5usTimerExpired)();
  /// @}
};

struct Controller_Host {
  struct HostTransport_Handle handle; ///< device specific pimpl

  void (*doTakeBytes)(uint8_t *, uint16_t); ///< Device API for host-transport module.

  /**
   * Device API for Host-Transport callbacks upon doTakeBytes(uint8_t *, uint16_t).
   *
   * @{
   */
  int (*onRequestGetFirmwareVersion)();
  int (*onRequestGetOutputDataRate)();
  int (*onRequestSetOutputDatatRate)(enum TransportRx_SetOutputDataRate_Rate);
  int (*onRequestGetRange)();
  int (*onRequestSetRange)(enum TransportRx_SetRange_Range);
  int (*onRequestGetScale)();
  int (*onRequestSetScale)(enum TransportRx_SetScale_Scale);
  int (*onRequestGetDeviceSetup)();
  int (*onRequestSamplingStart)(uint16_t);
  int (*onRequestSamplingStop)();
  /// @}
};

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

  struct Controller_Sensor sensor; ///< Device API for sensor and sensor pimpl.
  struct Controller_Sampling sampling; ///< Device API for sampling and sampling pimpl.
  struct Controller_Host host; ///< Device API for host transport and host transport pimpl.

  /**
   * Basic device API.
   * @{
   */
  void (*init)();
  void (*loop)();

  void (*checkReboot)();
  void (*requestReboot)();
  ///< @}
};
