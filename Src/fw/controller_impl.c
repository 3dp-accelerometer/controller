/**
 * \file device_impl.c
 *
 * Implements device API.
 */

#include "fw/controller_impl.h"
#include "fw/adxl345_transport_impl.h"
#include "fw/host_transport_impl.h"
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
static_assert(
  SAMPLING_NUM_SAMPLES_READ_AT_ONCE <= ADXL345_WATERMARK_LEVEL,
  "ERROR: maximum allowed read-at-once buffer: "
  MYSTRINGIZE( ADXL345_WATERMARK_LEVEL));

static_assert(
  TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER == ADXL345_WATERMARK_LEVEL,
  "ERROR: TransportTx transmit buffer and ADXL345 watermark level must be same: "
  MYSTRINGIZE(TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER) " vs. "
  MYSTRINGIZE(ADXL345_WATERMARK_LEVEL));
// clang-format on

#undef MYSTRINGIZE
#undef MYSTRINGIZE0

struct Controller_Handle controllerHandle = {
    .swVersionMajor = VERSION_MAJOR,
    .swVersionMinor = VERSION_MINOR,
    .swVersionPatch = VERSION_PATCH,

    .sensor = {.handle = ADXL345_HANDLE_INITIALIZER,
               .init = ControllerImpl_sensor_Adxl345_init},

    .sampling =
        {
            .handle = SAMPLING_DECLARE_INITIALIZER,
            .doSetFifoWatermark = ControllerImpl_sampling_setFifoWatermark,
            .doClearFifoWatermark = ControllerImpl_sampling_clearFifoWatermark,
            .doSetFifoOverflow = ControllerImpl_sampling_setFifoOverflow,
            .doSet5usTimerExpired = ControllerImpl_sampling_on5usTimerExpired,

        },

    .host =
        {
            .handle = HOSTTRANSPORT_DECLARE_INITIALIZER,
            .doTakeBytes = ControllerImpl_host_doTakeBytes,
            .onRequestGetFirmwareVersion =
                ControllerImpl_host_onRequestGetFirmwareVersion,
            .onRequestGetOutputDataRate =
                ControllerImpl_host_onRequestGetOutputDataRate,
            .onRequestSetOutputDatatRate =
                ControllerImpl_host_onRequestSetOutputDatatRate,
            .onRequestGetRange = ControllerImpl_host_onRequestGetRange,
            .onRequestSetRange = ControllerImpl_host_onRequestSetRange,
            .onRequestGetScale = ControllerImpl_host_onRequestGetScale,
            .onRequestSetScale = ControllerImpl_host_onRequestSetScale,
            .onRequestGetDeviceSetup =
                ControllerImpl_host_onRequestGetDeviceSetup,
            .onRequestSamplingStart =
                ControllerImpl_host_onRequestSamplingStart,
            .onRequestSamplingStop = ControllerImpl_host_onRequestSamplingStop,
        },

    .init = ControllerImpl_init,
    .loop = ControllerImpl_loop,

    .checkReboot = ControllerImpl_device_checkReboot,
    .requestReboot = ControllerImpl_device_requestAsyncReboot};

/* Device ------------------------------------------------------------------- */
/**
 * Flag indicating a device reboot was requested.
 *
 * \see ControllerImpl_device_requestAsyncReboot()
 */
static bool Controller_rebootRequested = false;

void ControllerImpl_init() { controllerHandle.sensor.init(); }

void ControllerImpl_loop() {
  switch (Sampling_fetchForward(&controllerHandle.sampling.handle)) {
  case -ECANCELED:
  case -EOVERFLOW:
    HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_RESET);
    break;
  case ENODATA:
    HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_SET);
    break;
  }
  controllerHandle.checkReboot();
}

void ControllerImpl_device_checkReboot() {
  if (Controller_rebootRequested) {
    NVIC_SystemReset();
  }
}

void ControllerImpl_device_requestAsyncReboot() {
  Controller_rebootRequested = true;
}

/* Host RX data ------------------------------------------------------------- */

void ControllerImpl_host_doTakeBytes(uint8_t *buffer, uint16_t len) {
  TransportRx_Process(&controllerHandle.host.handle, buffer, len);
}

int ControllerImpl_host_onRequestGetFirmwareVersion() {

  struct TransportFrame response = {
      .header.id = Transport_HeaderId_Tx_FirmwareVersion,
      .asTxFrame.asFirmwareVersion.major = controllerHandle.swVersionMajor,
      .asTxFrame.asFirmwareVersion.minor = controllerHandle.swVersionMinor,
      .asTxFrame.asFirmwareVersion.patch = controllerHandle.swVersionPatch};

  // send version
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             &controllerHandle.host.handle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asFirmwareVersion)))
    ;

  return 0;
}

int ControllerImpl_host_onRequestGetOutputDataRate() {
  enum Adxl345Flags_BwRate_Rate rate;
  Adxl345_getOutputDataRate(&controllerHandle.sensor.handle, &rate);

  struct TransportFrame response = {.header.id =
                                        Transport_HeaderId_Tx_OutputDataRate};
  response.asTxFrame.asOutputDataRate.rate =
      (enum TransportRx_SetOutputDataRate_Rate)rate;
  // send ODR
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             &controllerHandle.host.handle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asOutputDataRate)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetOutputDatatRate(
    enum TransportRx_SetOutputDataRate_Rate odr) {
  return Adxl345_setOutputDataRate(&controllerHandle.sensor.handle, odr);
}

int ControllerImpl_host_onRequestGetRange() {
  enum Adxl345Flags_DataFormat_Range range;
  Adxl345_getRange(&controllerHandle.sensor.handle, &range);

  struct TransportFrame response = {.header.id = Transport_HeaderId_Tx_Range};
  response.asTxFrame.asRange.range = (enum TransportRx_SetRange_Range)range;

  // send range
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             &controllerHandle.host.handle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asRange)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetRange(
    enum TransportRx_SetRange_Range range) {
  return Adxl345_setRange(&controllerHandle.sensor.handle, range);
}

int ControllerImpl_host_onRequestGetScale() {
  enum Adxl345Flags_DataFormat_FullResBit scale;
  Adxl345_getScale(&controllerHandle.sensor.handle, &scale);

  struct TransportFrame response = {.header.id = Transport_HeaderId_Tx_Scale};
  response.asTxFrame.asScale.scale = (enum TransportRx_SetScale_Scale)scale;

  // send scale
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             &controllerHandle.host.handle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asScale)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetScale(
    enum TransportRx_SetScale_Scale scale) {
  return Adxl345_setScale(&controllerHandle.sensor.handle, scale);
}

int ControllerImpl_host_onRequestGetDeviceSetup() {
  enum Adxl345Flags_BwRate_Rate rate;
  enum Adxl345Flags_DataFormat_Range range;
  enum Adxl345Flags_DataFormat_FullResBit scale;

  Adxl345_getOutputDataRate(&controllerHandle.sensor.handle, &rate);
  Adxl345_getRange(&controllerHandle.sensor.handle, &range);
  Adxl345_getScale(&controllerHandle.sensor.handle, &scale);

  struct TransportFrame response = {.header.id =
                                        Transport_HeaderId_Tx_DeviceSetup};
  response.asTxFrame.asDeviceSetup.outputDataRate =
      (enum TransportRx_SetOutputDataRate_Rate)rate;
  response.asTxFrame.asDeviceSetup.range =
      (enum TransportRx_SetRange_Range)range;
  response.asTxFrame.asDeviceSetup.scale =
      (enum TransportRx_SetScale_Scale)scale;

  // send scale
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             &controllerHandle.host.handle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asDeviceSetup)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSamplingStart(uint16_t maxSamplesCount) {
  Sampling_start(&controllerHandle.sampling.handle, maxSamplesCount);
  return 0;
}

int ControllerImpl_host_onRequestSamplingStop() {
  Sampling_stop(&controllerHandle.sampling.handle);
  return 0;
}

/* Sensor ------------------------------------------------------------------- */

void ControllerImpl_sensor_Adxl345_init() {
  Adxl345_init(&controllerHandle.sensor.handle);
}

int ControllerImpl_sensor_Adxl345_getOutputDataRate(uint8_t *odr) {
  enum Adxl345Flags_BwRate_Rate orate;
  int ret = Adxl345_getOutputDataRate(&controllerHandle.sensor.handle, &orate);
  *odr = orate;
  return ret;
}

int ControllerImpl_sensor_Adxl345_getScale(uint8_t *scale) {
  enum Adxl345Flags_DataFormat_FullResBit scl;
  int ret = Adxl345_getScale(&controllerHandle.sensor.handle, &scl);
  *scale = scl;
  return ret;
}

int ControllerImpl_sensor_Adxl345_getRange(uint8_t *range) {
  enum Adxl345Flags_DataFormat_Range rng;
  int ret = Adxl345_getRange(&controllerHandle.sensor.handle, &rng);
  *range = rng;
  return ret;
}

void ControllerImpl_sampling_setFifoWatermark() {
  Sampling_setFifoWatermark(&controllerHandle.sampling.handle);
}

void ControllerImpl_sampling_clearFifoWatermark() {
  Sampling_clearFifoWatermark(&controllerHandle.sampling.handle);
}

void ControllerImpl_sampling_setFifoOverflow() {
  Sampling_setFifoOverflow(&controllerHandle.sampling.handle);
}

void ControllerImpl_sampling_on5usTimerExpired() {
  Sampling_on5usTimerExpired(&controllerHandle.sampling.handle);
}

void ControllerImpl_sampling_onSamplingStarted() {
  TransportTx_FirmwareVersion(&controllerHandle.host.handle);
  TransportTx_SamplingStarted(
      &controllerHandle.host.handle,
      controllerHandle.sampling.handle.state.maxSamples);
}

void ControllerImpl_sampling_onSamplingStopped() {
  TransportTx_SamplingStopped(&controllerHandle.host.handle);
}

void ControllerImpl_sampling_onSamplingAborted() {
  TransportTx_SamplingAborted(&controllerHandle.host.handle);
}

void ControllerImpl_sampling_onSamplingFinished() {
  TransportTx_SamplingFinished(&controllerHandle.host.handle);
}

void ControllerImpl_sampling_doForwardAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex) {

  // type conversion in favour of loose dependencies in between transport- and
  // sampling-module.
  static_assert(sizeof(struct Sampling_Acceleration) ==
                    sizeof(struct Transport_Acceleration),
                "Error: acceleration structs must match in size!");

  TransportTx_AccelerationBuffer(&controllerHandle.host.handle,
                                 (const struct Transport_Acceleration *)buffer,
                                 bufferLen, startIndex);
}
void ControllerImpl_sampling_onFifoOverflow() {
  TransportTx_FifoOverflow(&controllerHandle.host.handle);
}
void ControllerImpl_sampling_doEnableSensor() {
  Adxl345_setPowerCtlMeasure(&controllerHandle.sensor.handle);
}
void ControllerImpl_sampling_doDisableSensor() {
  Adxl345_setPowerCtlStandby(&controllerHandle.sensor.handle);
}
void ControllerImpl_sampling_doFetchSensorAcceleration(
    struct Sampling_Acceleration *sample) {
  struct Adxl345Transport_Acceleration sensorSample;
  Adxl345_getAcceleration(&controllerHandle.sensor.handle, &sensorSample);

  if (NULL != sample) {
    sample->x = sensorSample.x;
    sample->y = sensorSample.y;
    sample->z = sensorSample.z;
  }
}
