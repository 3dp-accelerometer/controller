/**
 * \file device_impl.c
 *
 * Implements hardware specific controller implementation API.
 */

#include "fw/adxl345_transport_impl.h"
#include "fw/host_transport_impl.h"
#include "fw/led.h"
#include "fw/ringbuffer_impl.h"
#include "fw/sampling_impl.h"
#include "fw/version.h"
#include "main.h"
#include <adxl345.h>
#include <adxl345_flags.h>
#include <adxl345_transport_types.h>
#include <controller.h>
#include <errno.h>
#include <from_host_transport.h>
#include <host_transport_types.h>
#include <sampling.h>
#include <sampling_types.h>
#include <stm32f4xx_hal.h>
#include <to_host_transport.h>

#define MYSTRINGIZE0(A) #A
#define MYSTRINGIZE(A) MYSTRINGIZE0(A)

// clang-format off

// NOLINTNEXTLINE(readability-redundant-declaration)
static_assert(
  SAMPLING_NUM_SAMPLES_READ_AT_ONCE <= ADXL345_WATERMARK_LEVEL,
  "ERROR: maximum allowed read-at-once buffer: "
  MYSTRINGIZE( ADXL345_WATERMARK_LEVEL));

// NOLINTNEXTLINE(readability-redundant-declaration)
static_assert(
  TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES == ADXL345_WATERMARK_LEVEL,
  "ERROR: TransportTx transmit buffer and ADXL345 watermark level must be same: "
  MYSTRINGIZE(TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES) " vs. "
  MYSTRINGIZE(ADXL345_WATERMARK_LEVEL));

// clang-format on

#undef MYSTRINGIZE
#undef MYSTRINGIZE0

/**
 * Controller public API implementation.
 *
 * @{
 */
static void ControllerImpl_init();
static void ControllerImpl_loop();
/// @}

/**
 * Device specific controller implementation.
 *
 * @{
 */
static void ControllerImpl_device_checkReboot();
static void ControllerImpl_device_requestAsyncReboot();
static void ControllerImpl_transmitPendingResponses();
/// @}

/**
 * Device specific controller implementation for host communication.
 *
 * @{
 */
static void host_doTakeBytes(const uint8_t *buffer, uint16_t len);
static void host_onRequestGetFirmwareVersion();
static void host_responseGetFirmwareVersion();
static void host_onRequestGetOutputDataRate();
static void host_responseGetOutputDataRate();
static int
host_onRequestSetOutputDatatRate(enum TransportRx_SetOutputDataRate_Rate odr);
static void host_onRequestGetRange();
static void host_responseGetRange();
static int host_onRequestSetRange(enum TransportRx_SetRange_Range range);
static void host_onRequestGetScale();
static void host_responseGetScale();
static int host_onRequestSetScale(enum TransportRx_SetScale_Scale scale);
static void host_onRequestGetDeviceSetup();
static void host_responseGetDeviceSetup();
static void host_onRequestSamplingStart(uint16_t maxSamplesCount);
static void host_onRequestSamplingStop();
static void host_onRequestGetUptime();
static void host_responseGetUptime();
static void host_onRequestGetBufferStatus();
static void host_responseGetBufferStatus();
static void sampling_onTransmissionErrorCb();
static void sampling_responseTransmissionError();
/// @}

/**
 * Device specific controller implementation for acceleration sensor
 * manipulation.
 *
 * @{
 */
static void sensor_doInitImpl();
static int sensor_doGetOutputDataRateImpl(uint8_t *odr);
static int sensor_doGetScaleImpl(uint8_t *scale);
static int sensor_doGetRangeImpl(uint8_t *range);
/// @}

/**
 * Device specific controller implementation for the sampling module.
 *
 * @{
 */
static void sampling_setFifoWatermark();
static void sampling_clearFifoWatermark();
static void sampling_setFifoOverflow();
static void sampling_on5usTimerExpired();
static void sampling_onSamplingStartedCb();
static void sampling_onSamplingStoppedCb();
static void sampling_responseSamplingStopped();
static void sampling_onSamplingAbortedCb();
static void sampling_responseSamplingAborted();
static void sampling_onSamplingFinishedCb();
static void sampling_responseSamplingFinished();
static int sampling_doForwardAccelerationBufferImpl(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t firstIndex);
static void sampling_onFifoOverflowCb();
static void sampling_responseFifoOverflow();
static void sampling_onBufferOverflowCb();
static void sampling_responseBufferOverflow();
static void sampling_doEnableSensorImpl();
static void sampling_doDisableSensorImpl();
static void
sampling_doFetchSensorAccelerationImpl(struct Sampling_Acceleration *sample);
/// @}

/**
 * Device specific controller implementation for fault handlers.
 * @{
 */
static void fault_onNmiFaultHandler();
static void fault_onUsageFaultHandler();
static void fault_onBusFaultHandler();
static void fault_onHardFaultHandler();
static void fault_onErrorHandler();
/// @}

struct Transport_ResponseFlags {
  uint8_t host_responseGetFirmwareVersion : 1;
  uint8_t host_responseGetOutputDataRate : 1;
  uint8_t host_responseGetRange : 1;
  uint8_t host_responseGetScale : 1;
  uint8_t host_responseGetDeviceSetup : 1;
  uint8_t host_responseGetUptime : 1;
  uint8_t host_responseGetBufferStatus : 1;
  uint8_t sampling_responseSamplingStopped : 1;
  uint8_t sampling_responseSamplingAborted : 1;
  uint8_t sampling_responseSamplingFinished : 1;
  uint8_t sampling_responseFifoOverflow : 1;
  uint8_t sampling_responseBufferOverflow : 1;
  uint8_t sampling_responseTransmissionError : 1;
  uint8_t _reserved_0 : 1;
  uint8_t _reserved_1 : 1;
} __attribute__((packed));

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
volatile struct Transport_ResponseFlags pendingResponses = {0};

#define SAMPLING_DECLARE_INITIALIZER                                           \
  {                                                                            \
    .state = {.maxSamples = 0,                                                 \
              .doStart = false,                                                \
              .doStop = false,                                                 \
              .isStarted = false,                                              \
              .waitFor5usTimer = false,                                        \
              .rxBuffer = {{.x = 0, .y = 0, .z = 0}},                          \
              .isFifoOverflowSet = false,                                      \
              .isFifoWatermarkSet = false,                                     \
              .transactionsCount = 0},                                         \
                                                                               \
    .doEnableSensorImpl = sampling_doEnableSensorImpl,                         \
    .doDisableSensorImpl = sampling_doDisableSensorImpl,                       \
    .doFetchSensorAccelerationImpl = sampling_doFetchSensorAccelerationImpl,   \
    .doWaitDelay5usImpl = SamplingImpl_doWaitDelay5usImpl,                     \
    .doForwardAccelerationBufferImpl =                                         \
        sampling_doForwardAccelerationBufferImpl,                              \
                                                                               \
    .onSamplingStartedCb = sampling_onSamplingStartedCb,                       \
    .onSamplingStoppedCb = sampling_onSamplingStoppedCb,                       \
    .onSamplingAbortedCb = sampling_onSamplingAbortedCb,                       \
    .onSamplingFinishedCb = sampling_onSamplingFinishedCb,                     \
    .onFifoOverflowCb = sampling_onFifoOverflowCb,                             \
    .onBufferOverflowCb = sampling_onBufferOverflowCb,                         \
    .onTransmissionErrorCb = sampling_onTransmissionErrorCb,                   \
  }

#define HOSTTRANSPORT_DECLARE_INITIALIZER                                      \
  {                                                                            \
    .fromHost = {.doTakeReceivedPacketImpl =                                   \
                     HostTransportImpl_onTakeReceivedImpl},                    \
    .toHost = {                                                                \
      .ringbuffer = RINGBUFFER_DECLARE_INITIALIZER,                            \
      .largestTxChunkBytes = 0,                                                \
      .doTransmitImpl = HostTransportImpl_doTransmitImpl,                      \
      .isTransmitBusyImpl = HostTransportImpl_isTransmitBusyImpl,              \
    }                                                                          \
  }

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
struct Controller_Handle controllerHandle = {
    .sensor = {.handle = ADXL345_HANDLE_INITIALIZER, .init = sensor_doInitImpl},

    .sampling =
        {
            .handle = SAMPLING_DECLARE_INITIALIZER,
            .doSetFifoWatermark = sampling_setFifoWatermark,
            .doClearFifoWatermark = sampling_clearFifoWatermark,
            .doSetFifoOverflow = sampling_setFifoOverflow,
            .doSet5usTimerExpired = sampling_on5usTimerExpired,

        },

    .host =
        {
            .handle = HOSTTRANSPORT_DECLARE_INITIALIZER,
            .doTakeBytes = host_doTakeBytes,
            .onRequestGetFirmwareVersion = host_onRequestGetFirmwareVersion,
            .onRequestGetOutputDataRate = host_onRequestGetOutputDataRate,
            .onRequestSetOutputDatatRate = host_onRequestSetOutputDatatRate,
            .onRequestGetRange = host_onRequestGetRange,
            .onRequestSetRange = host_onRequestSetRange,
            .onRequestGetScale = host_onRequestGetScale,
            .onRequestSetScale = host_onRequestSetScale,
            .onRequestGetDeviceSetup = host_onRequestGetDeviceSetup,
            .onRequestSamplingStart = host_onRequestSamplingStart,
            .onRequestSamplingStop = host_onRequestSamplingStop,
            .onRequestUptime = host_onRequestGetUptime,
            .onRequestBufferStatus = host_onRequestGetBufferStatus,
        },

    .init = ControllerImpl_init,
    .loop = ControllerImpl_loop,

    .requestReboot = ControllerImpl_device_requestAsyncReboot,
    .fault_onNmiFaultHandler = fault_onNmiFaultHandler,
    .fault_onUsageFaultHandler = fault_onUsageFaultHandler,
    .fault_onBusFaultHandler = fault_onBusFaultHandler,
    .fault_onHardFaultHandler = fault_onHardFaultHandler,
    .fault_onErrorHandler = fault_onErrorHandler,
};

/* Device ------------------------------------------------------------------- */

/**
 * Flag indicating a device reboot was requested.
 *
 * \see ControllerImpl_device_requestAsyncReboot()
 *
 * @{
 */
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static bool rebootRequested = false;
/// @}

void ControllerImpl_init() { controllerHandle.sensor.init(); }

void ControllerImpl_loop() {
  switch (Sampling_fetchForward(&controllerHandle.sampling.handle)) {
  case -ECANCELED: // NOLINT(bugprone-branch-clone)
  case -EOVERFLOW: // NOLINT(bugprone-branch-clone)
    USER_LED0_ON;
    break;
  case ENODATA:
    USER_LED0_OFF;
    break;
  default:
    break;
  }
  ControllerImpl_device_checkReboot();
  ControllerImpl_transmitPendingResponses();
}

void ControllerImpl_device_checkReboot() {
  if (rebootRequested) {
    NVIC_SystemReset();
  }
}

void ControllerImpl_device_requestAsyncReboot() { rebootRequested = true; }

/**
 * Decouples possible interrupt context from execution which is performed in
 * in main() context.
 *
 * Example: each user request, handled in ISR, sets a flag so that the
 * it can be handled later in context of main().
 */
static void ControllerImpl_transmitPendingResponses() {

  if (pendingResponses.host_responseGetFirmwareVersion) {
    pendingResponses.host_responseGetFirmwareVersion = false;
    host_responseGetFirmwareVersion();
  }
  if (pendingResponses.host_responseGetOutputDataRate) {
    pendingResponses.host_responseGetOutputDataRate = false;
    host_responseGetOutputDataRate();
  }
  if (pendingResponses.host_responseGetRange) {
    pendingResponses.host_responseGetRange = false;
    host_responseGetRange();
  }
  if (pendingResponses.host_responseGetScale) {
    pendingResponses.host_responseGetScale = false;
    host_responseGetScale();
  }
  if (pendingResponses.host_responseGetDeviceSetup) {
    pendingResponses.host_responseGetDeviceSetup = false;
    host_responseGetDeviceSetup();
  }
  if (pendingResponses.host_responseGetUptime) {
    pendingResponses.host_responseGetUptime = false;
    host_responseGetUptime();
  }
  if (pendingResponses.host_responseGetBufferStatus) {
    pendingResponses.host_responseGetBufferStatus = false;
    host_responseGetBufferStatus();
  }

  if (pendingResponses.sampling_responseSamplingStopped) {
    pendingResponses.sampling_responseSamplingStopped = false;
    sampling_responseSamplingStopped();
  }
  if (pendingResponses.sampling_responseSamplingAborted) {
    pendingResponses.sampling_responseSamplingAborted = false;
    sampling_responseSamplingAborted();
  }
  if (pendingResponses.sampling_responseSamplingFinished) {
    pendingResponses.sampling_responseSamplingFinished = false;
    sampling_responseSamplingFinished();
  }
  if (pendingResponses.sampling_responseFifoOverflow) {
    pendingResponses.sampling_responseFifoOverflow = false;
    sampling_responseFifoOverflow();
  }
  if (pendingResponses.sampling_responseBufferOverflow) {
    pendingResponses.sampling_responseBufferOverflow = false;
    sampling_responseBufferOverflow();
  }
  if (pendingResponses.sampling_responseTransmissionError) {
    pendingResponses.sampling_responseTransmissionError = false;
    sampling_responseTransmissionError();
  }
}

/* Host RX data ------------------------------------------------------------- */

/**
 * Handles incoming bytes (unfragmented data packet).
 *
 * Calls generic TransportRx_Process(uint8_t *buffer, uint16_t length)
 * implementation which performs basic checks only. Further
 * processing/dispatching is delegated to the respective pimpl. \see
 * host_transport_impl.h
 *
 * @param buffer received byte buffer (must not be fragmented)
 * @param len received data length
 */
static void host_doTakeBytes(const uint8_t *buffer, uint16_t len) {
  TransportRx_Process(&controllerHandle.host.handle, buffer, len);
}

static void host_onRequestGetFirmwareVersion() {
  pendingResponses.host_responseGetFirmwareVersion = true;
}

static void host_responseGetFirmwareVersion() {
  TransportTx_TxFirmwareVersion(&controllerHandle.host.handle, VERSION_MAJOR,
                                VERSION_MINOR, VERSION_PATCH);
}

static void host_onRequestGetOutputDataRate() {
  pendingResponses.host_responseGetOutputDataRate = true;
}

static void host_responseGetOutputDataRate() {
  uint8_t odr = {0};
  sensor_doGetOutputDataRateImpl(&odr);
  TransportTx_TxOutputDataRate(&controllerHandle.host.handle, odr);
}

static int
host_onRequestSetOutputDatatRate(enum TransportRx_SetOutputDataRate_Rate odr) {
  return Adxl345_setOutputDataRate(&controllerHandle.sensor.handle, odr);
}

static void host_onRequestGetRange() {
  pendingResponses.host_responseGetRange = true;
}

static void host_responseGetRange() {
  uint8_t range = {0};
  sensor_doGetRangeImpl(&range);
  TransportTx_TxRange(&controllerHandle.host.handle, range);
}

static int host_onRequestSetRange(enum TransportRx_SetRange_Range range) {
  return Adxl345_setRange(&controllerHandle.sensor.handle, range);
}

static void host_onRequestGetScale() {
  pendingResponses.host_responseGetScale = true;
}

static void host_responseGetScale() {
  uint8_t scale = {0};
  sensor_doGetScaleImpl(&scale);
  TransportTx_TxScale(&controllerHandle.host.handle, scale);
}

static int host_onRequestSetScale(enum TransportRx_SetScale_Scale scale) {
  return Adxl345_setScale(&controllerHandle.sensor.handle, scale);
}

static void host_onRequestGetDeviceSetup() {
  pendingResponses.host_responseGetDeviceSetup = true;
}

static void host_responseGetDeviceSetup() {
  uint8_t odr = {0};
  uint8_t scale = {0};
  uint8_t range = {0};

  sensor_doGetOutputDataRateImpl(&odr);
  sensor_doGetScaleImpl(&scale);
  sensor_doGetRangeImpl(&range);

  TransportTx_TxSamplingSetup(&controllerHandle.host.handle, odr, scale, range);
}

static void host_onRequestSamplingStart(uint16_t maxSamplesCount) {
  Sampling_start(&controllerHandle.sampling.handle, maxSamplesCount);
}

static void host_onRequestSamplingStop() {
  Sampling_stop(&controllerHandle.sampling.handle);
}

static void host_onRequestGetUptime() {
  pendingResponses.host_responseGetUptime = true;
}

static void host_responseGetUptime() {
  TransportTx_TxUptime(&controllerHandle.host.handle, HAL_GetTick());
}

static void host_onRequestGetBufferStatus() {
  pendingResponses.host_responseGetBufferStatus = true;
}

static void host_responseGetBufferStatus() {
  TransportTx_TxBufferStatus(
      &controllerHandle.host.handle, RINGBUFFER_STORAGE_SIZE_BYTES,
      RINGBUFFER_STORAGE_ITEMS,
      Ringbuffer_maxCapacityUsed(
          &controllerHandle.host.handle.toHost.ringbuffer),
      Ringbuffer_putCount(&controllerHandle.host.handle.toHost.ringbuffer),
      Ringbuffer_takeCount(&controllerHandle.host.handle.toHost.ringbuffer),
      controllerHandle.host.handle.toHost.largestTxChunkBytes);
}

/* Sensor ------------------------------------------------------------------- */

static void sensor_doInitImpl() {
  Adxl345_init(&controllerHandle.sensor.handle);
}

static int sensor_doGetOutputDataRateImpl(uint8_t *odr) {
  enum Adxl345Flags_BwRate_Rate adxlOdr = {0};
  int ret =
      Adxl345_getOutputDataRate(&controllerHandle.sensor.handle, &adxlOdr);
  *odr = adxlOdr;
  return ret;
}

static int sensor_doGetScaleImpl(uint8_t *scale) {
  enum Adxl345Flags_DataFormat_FullResBit adxlScale = {0};
  int ret = Adxl345_getScale(&controllerHandle.sensor.handle, &adxlScale);
  *scale = adxlScale;
  return ret;
}

static int sensor_doGetRangeImpl(uint8_t *range) {
  enum Adxl345Flags_DataFormat_Range adxlRange = {0};
  int ret = Adxl345_getRange(&controllerHandle.sensor.handle, &adxlRange);
  *range = adxlRange;
  return ret;
}

static void sampling_setFifoWatermark() {
  Sampling_setFifoWatermark(&controllerHandle.sampling.handle);
}

static void sampling_clearFifoWatermark() {
  Sampling_clearFifoWatermark(&controllerHandle.sampling.handle);
}

static void sampling_setFifoOverflow() {
  Sampling_setFifoOverflow(&controllerHandle.sampling.handle);
}

static void sampling_on5usTimerExpired() {
  Sampling_on5usTimerExpired(&controllerHandle.sampling.handle);
}

static void sampling_onSamplingStartedCb() {
  Transport_resetBuffer(&controllerHandle.host.handle);

  TransportTx_TxSamplingStarted(
      &controllerHandle.host.handle,
      controllerHandle.sampling.handle.state.maxSamples);
}

static void sampling_onSamplingStoppedCb() {
  pendingResponses.sampling_responseSamplingStopped = true;
}

static void sampling_responseSamplingStopped() {
  host_responseGetFirmwareVersion();
  host_responseGetBufferStatus();
  host_responseGetDeviceSetup();
  TransportTx_TxSamplingStopped(&controllerHandle.host.handle);
}

static void sampling_onSamplingAbortedCb() {
  pendingResponses.sampling_responseSamplingAborted = true;
}

static void sampling_responseSamplingAborted() {
  TransportTx_TxSamplingAborted(&controllerHandle.host.handle);
}

static void sampling_onSamplingFinishedCb() {
  pendingResponses.sampling_responseSamplingFinished = true;
}

static void sampling_responseSamplingFinished() {
  TransportTx_TxSamplingFinished(&controllerHandle.host.handle);
}

static int sampling_doForwardAccelerationBufferImpl(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t firstIndex) {

  // ugly c-style type conversion in favour of no dependency in between
  // transport-module and sampling-module

  static_assert(sizeof(struct Sampling_Acceleration) ==
                    sizeof(struct Transport_Acceleration),
                "ERROR: acceleration structs must match in size!");

  return TransportTx_TxAccelerationBuffer(
      &controllerHandle.host.handle,
      (const struct Transport_Acceleration *)buffer, bufferLen, firstIndex);
}

static void sampling_onFifoOverflowCb() {
  pendingResponses.sampling_responseFifoOverflow = true;
}

static void sampling_responseFifoOverflow() {
  TransportTx_TxFifoOverflow(&controllerHandle.host.handle);
}

static void sampling_onBufferOverflowCb() {
  pendingResponses.host_responseGetBufferStatus = true;
}

static void sampling_responseBufferOverflow() {
  TransportTx_TxBufferOverflow(&controllerHandle.host.handle);
}

static void sampling_onTransmissionErrorCb() {
  pendingResponses.sampling_responseTransmissionError = true;
}

static void sampling_responseTransmissionError() {
  TransportTx_TxTransmissionError(&controllerHandle.host.handle);
}

static void sampling_doEnableSensorImpl() {
  Adxl345_setPowerCtlMeasure(&controllerHandle.sensor.handle);
}

static void sampling_doDisableSensorImpl() {
  Adxl345_setPowerCtlStandby(&controllerHandle.sensor.handle);
}

static void
sampling_doFetchSensorAccelerationImpl(struct Sampling_Acceleration *sample) {
  struct Adxl345Transport_Acceleration sensorSample;
  Adxl345_getAcceleration(&controllerHandle.sensor.handle, &sensorSample);

  if (NULL != sample) {
    sample->x = sensorSample.x;
    sample->y = sensorSample.y;
    sample->z = sensorSample.z;
  }
}

static void fault_onNmiFaultHandler() {
  TransportTx_TxFault(&controllerHandle.host.handle,
                      TransportTx_FaultCode_NmiHandler);
}

static void fault_onUsageFaultHandler() {
  TransportTx_TxFault(&controllerHandle.host.handle,
                      TransportTx_FaultCode_UsageFaultHandler);
}

static void fault_onBusFaultHandler() {
  TransportTx_TxFault(&controllerHandle.host.handle,
                      TransportTx_FaultCode_BusFaultHandler);
}

static void fault_onHardFaultHandler() {
  USER_LED0_ON;
  // most likely usb will not transmit anything while in hard-fault handler
  TransportTx_TxFault(&controllerHandle.host.handle,
                      TransportTx_FaultCode_HardFaultHandler);
}

static void fault_onErrorHandler() {
  TransportTx_TxFault(&controllerHandle.host.handle,
                      TransportTx_FaultCode_ErrorHandler);
}
