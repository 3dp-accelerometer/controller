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
#include "usbd_cdc_if.h"
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

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

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
  TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER == ADXL345_WATERMARK_LEVEL,
  "ERROR: TransportTx transmit buffer and ADXL345 watermark level must be same: "
  MYSTRINGIZE(TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER) " vs. "
  MYSTRINGIZE(ADXL345_WATERMARK_LEVEL));

// clang-format on

#undef MYSTRINGIZE
#undef MYSTRINGIZE0

void ControllerImpl_init();
void ControllerImpl_loop();

void ControllerImpl_device_checkReboot();
void ControllerImpl_device_requestAsyncReboot();

static void host_doTakeBytes(uint8_t *buffer, uint16_t len);
static int host_onRequestGetFirmwareVersion();
static int host_onRequestGetOutputDataRate();
static int
host_onRequestSetOutputDatatRate(enum TransportRx_SetOutputDataRate_Rate odr);
static int host_onRequestGetRange();
static int host_onRequestSetRange(enum TransportRx_SetRange_Range range);
static int host_onRequestGetScale();
static int host_onRequestSetScale(enum TransportRx_SetScale_Scale scale);
static int host_onRequestGetDeviceSetup();
static int host_onRequestSamplingStart(uint16_t maxSamplesCount);
static int host_onRequestSamplingStop();
static int host_onRequestGetUptime();
static int host_onRequestGetBufferStatus();

static void sensor_Adxl345_doInitImpl();
static int sensor_Adxl345_doGetOutputDataRateImpl(uint8_t *odr);
static int sensor_Adxl345_doGetScaleImpl(uint8_t *scale);
static int sensor_Adxl345_doGetRangeImpl(uint8_t *range);

static void sampling_setFifoWatermark();
static void sampling_clearFifoWatermark();
static void sampling_setFifoOverflow();
static void sampling_on5usTimerExpired();
static void sampling_onSamplingStartedCb();
static void sampling_onSamplingStoppedCb();
static void sampling_onSamplingAbortedCb();
static void sampling_onSamplingFinishedCb();
static void sampling_doForwardAccelerationBufferImpl(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t firstIndex);
static void sampling_onFifoOverflowCb();
static void sampling_doEnableSensorImpl();
static void sampling_doDisableSensorImpl();
static void
sampling_doFetchSensorAccelerationImpl(struct Sampling_Acceleration *sample);

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
  }

#define HOSTTRANSPORT_DECLARE_INITIALIZER                                      \
  {                                                                            \
    .fromHost = {.doTakeReceivedPacketImpl =                                   \
                     HostTransportImpl_onTakeReceivedImpl},                    \
    .toHost = {                                                                \
      .txBuffer = UserTxBufferFS,                                              \
      .txBufferSize = APP_TX_DATA_SIZE,                                        \
      .ringbuffer = RINGBUFFER_DECLARE_INITIALIZER,                            \
      .ringbufferMaxItemsUtilization = 0,                                      \
      .doTransmitImpl = HostTransportImpl_doTransmitImpl,                      \
    }                                                                          \
  }

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
struct Controller_Handle controllerHandle = {
    .swVersionMajor = VERSION_MAJOR,
    .swVersionMinor = VERSION_MINOR,
    .swVersionPatch = VERSION_PATCH,

    .sensor = {.handle = ADXL345_HANDLE_INITIALIZER,
               .init = sensor_Adxl345_doInitImpl},

    .sampling =
        {
            .handle = SAMPLING_DECLARE_INITIALIZER,
            .doSetFifoWatermark = sampling_setFifoWatermark,
            .doClearFifoWatermark = sampling_clearFifoWatermark,
            .doSetFifoOverflow = sampling_setFifoOverflow,
            .doSet5usTimerExpired = sampling_on5usTimerExpired,

        },

    .host = {.handle = HOSTTRANSPORT_DECLARE_INITIALIZER,
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
             .onRequestBufferStatus = host_onRequestGetBufferStatus},

    .init = ControllerImpl_init,
    .loop = ControllerImpl_loop,

    .checkReboot = ControllerImpl_device_checkReboot,
    .requestReboot = ControllerImpl_device_requestAsyncReboot};

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
  controllerHandle.checkReboot();
}

void ControllerImpl_device_checkReboot() {
  if (rebootRequested) {
    NVIC_SystemReset();
  }
}

void ControllerImpl_device_requestAsyncReboot() { rebootRequested = true; }

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
static void host_doTakeBytes(uint8_t *buffer, uint16_t len) {
  TransportRx_Process(&controllerHandle.host.handle, buffer, len);
}

static int host_onRequestGetFirmwareVersion() {
  TransportTx_TxFirmwareVersion(&controllerHandle.host.handle, VERSION_MAJOR,
                                VERSION_MINOR, VERSION_PATCH);
  return 0;
}

static int host_onRequestGetOutputDataRate() {
  uint8_t odr = {0};
  sensor_Adxl345_doGetOutputDataRateImpl(&odr);
  TransportTx_TxOutputDataRate(&controllerHandle.host.handle, odr);
  return 0;
}

static int
host_onRequestSetOutputDatatRate(enum TransportRx_SetOutputDataRate_Rate odr) {
  return Adxl345_setOutputDataRate(&controllerHandle.sensor.handle, odr);
}

static int host_onRequestGetRange() {
  uint8_t range = {0};
  sensor_Adxl345_doGetRangeImpl(&range);
  TransportTx_TxRange(&controllerHandle.host.handle, range);
  return 0;
}

static int host_onRequestSetRange(enum TransportRx_SetRange_Range range) {
  return Adxl345_setRange(&controllerHandle.sensor.handle, range);
}

static int host_onRequestGetScale() {
  uint8_t scale = {0};
  sensor_Adxl345_doGetScaleImpl(&scale);
  TransportTx_TxScale(&controllerHandle.host.handle, scale);
  return 0;
}

static int host_onRequestSetScale(enum TransportRx_SetScale_Scale scale) {
  return Adxl345_setScale(&controllerHandle.sensor.handle, scale);
}

static int host_onRequestGetDeviceSetup() {
  uint8_t odr = {0};
  uint8_t scale = {0};
  uint8_t range = {0};

  sensor_Adxl345_doGetOutputDataRateImpl(&odr);
  sensor_Adxl345_doGetScaleImpl(&scale);
  sensor_Adxl345_doGetRangeImpl(&range);

  TransportTx_TxSamplingSetup(&controllerHandle.host.handle, odr, scale, range);
  return 0;
}

static int host_onRequestSamplingStart(uint16_t maxSamplesCount) {
  Sampling_start(&controllerHandle.sampling.handle, maxSamplesCount);
  return 0;
}

static int host_onRequestSamplingStop() {
  Sampling_stop(&controllerHandle.sampling.handle);
  return 0;
}

static int host_onRequestGetUptime() {
  uint32_t tickMs = {HAL_GetTick()};
  TransportTx_TxUptime(&controllerHandle.host.handle, tickMs);
  return 0;
}

static int host_onRequestGetBufferStatus() {
  TransportTx_BufferStatus(
      &controllerHandle.host.handle, RINGBUFFER_STORAGE_SIZE_BYTES,
      RINGBUFFER_STORAGE_ITEMS,
      controllerHandle.host.handle.toHost.ringbufferMaxItemsUtilization);
  return 0;
}

/* Sensor ------------------------------------------------------------------- */

static void sensor_Adxl345_doInitImpl() {
  Adxl345_init(&controllerHandle.sensor.handle);
}

static int sensor_Adxl345_doGetOutputDataRateImpl(uint8_t *odr) {
  enum Adxl345Flags_BwRate_Rate adxlOdr = {0};
  int ret =
      Adxl345_getOutputDataRate(&controllerHandle.sensor.handle, &adxlOdr);
  *odr = adxlOdr;
  return ret;
}

static int sensor_Adxl345_doGetScaleImpl(uint8_t *scale) {
  enum Adxl345Flags_DataFormat_FullResBit adxlScale = {0};
  int ret = Adxl345_getScale(&controllerHandle.sensor.handle, &adxlScale);
  *scale = adxlScale;
  return ret;
}

static int sensor_Adxl345_doGetRangeImpl(uint8_t *range) {
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
  TransportTx_TxSamplingStarted(
      &controllerHandle.host.handle,
      controllerHandle.sampling.handle.state.maxSamples);
}

static void sampling_onSamplingStoppedCb() {
  host_onRequestGetFirmwareVersion();
  host_onRequestGetBufferStatus();

  uint8_t odr = {0};
  uint8_t scale = {0};
  uint8_t range = {0};

  sensor_Adxl345_doGetOutputDataRateImpl(&odr);
  sensor_Adxl345_doGetScaleImpl(&scale);
  sensor_Adxl345_doGetRangeImpl(&range);

  TransportTx_TxSamplingStopped(&controllerHandle.host.handle, odr, scale,
                                range);
}

static void sampling_onSamplingAbortedCb() {
  TransportTx_TxSamplingAborted(&controllerHandle.host.handle);
}

static void sampling_onSamplingFinishedCb() {
  TransportTx_TxSamplingFinished(&controllerHandle.host.handle);
}

static void sampling_doForwardAccelerationBufferImpl(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t firstIndex) {

  // type conversion in favour of loose dependencies in between transport- and
  // sampling-module.
  static_assert(sizeof(struct Sampling_Acceleration) ==
                    sizeof(struct Transport_Acceleration),
                "Error: acceleration structs must match in size!");

  TransportTx_TxAccelerationBuffer(
      &controllerHandle.host.handle,
      (const struct Transport_Acceleration *)buffer, bufferLen, firstIndex);
}

static void sampling_onFifoOverflowCb() {
  TransportTx_TxFifoOverflow(&controllerHandle.host.handle);
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
