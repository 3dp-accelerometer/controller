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
#include <host_transport.h>
#include <host_transport_types.h>
#include <sampling.h>
#include <sampling_types.h>
#include <stm32f4xx_hal.h>
#include <to_host_transport.h>

ADXL345_DECLARE_HANDLE(sensorHandle);
SAMPLING_DECLARE_HANDLE(samplingHandle);
HOSTTRANSPORT_DECLARE_HANDLE(hostTransportHandle);
DEVICEIMPL_DECLARE_HANDLE(controllerHandle, sensorHandle, samplingHandle,
                          hostTransportHandle);

/* Device ------------------------------------------------------------------- */
/**
 * Flag indicating a device reboot was requested.
 *
 * \see ControllerImpl_device_requestAsyncReboot()
 */
static bool Controller_rebootRequested = false;

void ControllerImpl_init() { controllerHandle.sensorInit(); }

void ControllerImpl_loop() {
  switch (controllerHandle.samplingFetchForward()) {
  case -ECANCELED:
  case -EOVERFLOW:
    HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_RESET);
    break;
  case ENODATA:
    HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_SET);
    break;
  }
  controllerHandle.controllerCheckReboot();
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

void ControllerImpl_host_onBytesReceived(uint8_t *buffer, uint16_t len) {
  TransportRx_Process(controllerHandle.hostTransportHandle, buffer, len);
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
             controllerHandle.hostTransportHandle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asFirmwareVersion)))
    ;

  return 0;
}

int ControllerImpl_host_onRequestGetOutputDataRate() {
  enum Adxl345Flags_BwRate_Rate rate;
  Adxl345_getOutputDataRate(controllerHandle.sensorHandle, &rate);

  struct TransportFrame response = {.header.id =
                                        Transport_HeaderId_Tx_OutputDataRate};
  response.asTxFrame.asOutputDataRate.rate =
      (enum TransportRx_SetOutputDataRate_Rate)rate;
  // send ODR
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             controllerHandle.hostTransportHandle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asOutputDataRate)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetOutputDatatRate(
    enum TransportRx_SetOutputDataRate_Rate odr) {
  return Adxl345_setOutputDataRate(controllerHandle.sensorHandle, odr);
}

int ControllerImpl_host_onRequestGetRange() {
  enum Adxl345Flags_DataFormat_Range range;
  Adxl345_getRange(controllerHandle.sensorHandle, &range);

  struct TransportFrame response = {.header.id = Transport_HeaderId_Tx_Range};
  response.asTxFrame.asRange.range = (enum TransportRx_SetRange_Range)range;

  // send range
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             controllerHandle.hostTransportHandle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asRange)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetRange(
    enum TransportRx_SetRange_Range range) {
  return Adxl345_setRange(controllerHandle.sensorHandle, range);
}

int ControllerImpl_host_onRequestGetScale() {
  enum Adxl345Flags_DataFormat_FullResBit scale;
  Adxl345_getScale(controllerHandle.sensorHandle, &scale);

  struct TransportFrame response = {.header.id = Transport_HeaderId_Tx_Scale};
  response.asTxFrame.asScale.scale = (enum TransportRx_SetScale_Scale)scale;

  // send scale
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             controllerHandle.hostTransportHandle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asScale)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSetScale(
    enum TransportRx_SetScale_Scale scale) {
  return Adxl345_setScale(controllerHandle.sensorHandle, scale);
}

int ControllerImpl_host_onRequestGetDeviceSetup() {
  enum Adxl345Flags_BwRate_Rate rate;
  enum Adxl345Flags_DataFormat_Range range;
  enum Adxl345Flags_DataFormat_FullResBit scale;

  Adxl345_getOutputDataRate(controllerHandle.sensorHandle, &rate);
  Adxl345_getRange(controllerHandle.sensorHandle, &range);
  Adxl345_getScale(controllerHandle.sensorHandle, &scale);

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
             controllerHandle.hostTransportHandle, (uint8_t *)&response,
             SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asDeviceSetup)))
    ;
  return 0;
}

int ControllerImpl_host_onRequestSamplingStart(uint16_t maxSamplesCount) {
  controllerHandle.samplingStart(maxSamplesCount);
  return 0;
}

int ControllerImpl_host_onRequestSamplingStop() {
  controllerHandle.samplingStop();
  return 0;
}

/* Sensor ------------------------------------------------------------------- */

void ControllerImpl_sensor_Adxl345_init() {
  Adxl345_init(controllerHandle.sensorHandle);
}

void ControllerImpl_sampling_start(uint16_t maxSamples) {
  Sampling_start(controllerHandle.samplingHandle, maxSamples);
}

void ControllerImpl_sampling_stop() {
  Sampling_stop(controllerHandle.samplingHandle);
}

int ControllerImpl_sampling_fetchForward() {
  return Sampling_fetchForward(controllerHandle.samplingHandle);
}

void ControllerImpl_sampling_setFifoWatermark() {
  Sampling_setFifoWatermark(controllerHandle.samplingHandle);
}

void ControllerImpl_sampling_clearFifoWatermark() {
  Sampling_clearFifoWatermark(controllerHandle.samplingHandle);
}

void ControllerImpl_sampling_setFifoOverflow() {
  Sampling_setFifoOverflow(controllerHandle.samplingHandle);
}

void ControllerImpl_sampling_on5usTimerExpired() {
  Sampling_on5usTimerExpired(controllerHandle.samplingHandle);
}

void ControllerImpl_sampling_onSamplingStarted() {
  TransportTx_FirmwareVersion(controllerHandle.hostTransportHandle,
                              &controllerHandle);
  TransportTx_SamplingStarted(
      controllerHandle.hostTransportHandle,
      controllerHandle.samplingHandle->state.maxSamples);
}

void ControllerImpl_sampling_onSamplingStopped() {
  TransportTx_SamplingStopped(controllerHandle.hostTransportHandle,
                              controllerHandle.sensorHandle);
}

void ControllerImpl_sampling_onSamplingAborted() {
  TransportTx_SamplingAborted(controllerHandle.hostTransportHandle);
}

void ControllerImpl_sampling_onSamplingFinished() {
  TransportTx_SamplingFinished(controllerHandle.hostTransportHandle);
}

void ControllerImpl_sampling_onPostAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex) {
  TransportTx_AccelerationBuffer(controllerHandle.hostTransportHandle, buffer,
                                 bufferLen, startIndex);
}
void ControllerImpl_sampling_onFifoOverflow() {
  TransportTx_FifoOverflow(controllerHandle.hostTransportHandle);
}
void ControllerImpl_sampling_onSensorEnable() {
  Adxl345_setPowerCtlMeasure(controllerHandle.sensorHandle);
}
void ControllerImpl_sampling_onSensorDisable() {
  Adxl345_setPowerCtlStandby(controllerHandle.sensorHandle);
}
void ControllerImpl_sampling_onFetchSensorAcceleration(
    struct Sampling_Acceleration *sample) {
  struct Adxl345Transport_Acceleration sensorSample;
  Adxl345_getAcceleration(controllerHandle.sensorHandle, &sensorSample);

  if (NULL != sample) {
    sample->x = sensorSample.x;
    sample->y = sensorSample.y;
    sample->z = sensorSample.z;
  }
}
