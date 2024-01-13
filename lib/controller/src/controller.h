/**
 * \file controller.h
 *
 * The controller module aims at abstracting the whole device API.
 *
 * The device API consists of the following APIs:
 *   - host transport API
 *   - sensor API
 *   - sampling API
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

  void (*const init)(); ///< Context: main()
};

struct Controller_Sampling {
  struct Sampling_Handle handle; ///< device specific pimpl

  /**
   * Device API for sampling module.
   *
   * @{
   */
  void (*const doSetFifoWatermark)();   ///< Context: EXTI2_IRQHandler()
  void (*const doClearFifoWatermark)(); ///< Context: EXTI2_IRQHandler()
  void (*const doSetFifoOverflow)();    ///< Context: EXTI3_IRQHandler()
  void (*const doSet5usTimerExpired)(); ///< Context: TIM3_IRQHandler()
  /// @}
};

struct Controller_Host {
  struct HostTransport_Handle handle; ///< device specific pimpl

  /**
   * Device API for host-transport module.
   *
   * Context: CDC_Receive_FS(uint8_t*, uint32_t *)
   */
  void (*const doTakeBytes)(const uint8_t *, uint16_t);

  /**
   * Device API for Host-Transport callbacks upon doTakeBytes(uint8_t *,
   * uint16_t).
   *
   * Context: CDC_Receive_FS(uint8_t*, uint32_t *)
   * @{
   */
  int (*const onRequestGetFirmwareVersion)();
  int (*const onRequestGetOutputDataRate)();
  int (*const onRequestSetOutputDatatRate)(
      enum TransportRx_SetOutputDataRate_Rate);
  int (*const onRequestGetRange)();
  int (*const onRequestSetRange)(enum TransportRx_SetRange_Range);
  int (*const onRequestGetScale)();
  int (*const onRequestSetScale)(enum TransportRx_SetScale_Scale);
  int (*const onRequestGetDeviceSetup)();
  int (*const onRequestSamplingStart)(uint16_t);
  int (*const onRequestSamplingStop)();
  int (*const onRequestUptime)();
  int (*const onRequestBufferStatus)();
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
 */
struct Controller_Handle {
  /**
   * Device API for sensor and sensor pimpl.
   */
  struct Controller_Sensor sensor;

  /**
   * Device API for sampling and sampling pimpl.
   */
  struct Controller_Sampling sampling;

  /**
   * Device API for host transport and host transport pimpl.
   */
  struct Controller_Host host;

  /**
   * Public device API.
   * @{
   */
  void (*const init)(); ///< Context: main()
  void (*const loop)(); ///< Context: main()

  void (*const checkReboot)();   ///< Context: main()
  void (*const requestReboot)(); ///< Context: main()

  /// @}
};
