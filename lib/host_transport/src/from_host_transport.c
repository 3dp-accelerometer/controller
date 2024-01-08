/**
 * \file from_host_transport.c
 *
 * Implementation for processing data from host to controller.
 */

#include "host_transport.h"
#include "host_transport_types.h"
#include <adxl345.h>
#include <adxl345_flags.h>
#include <adxl345_transport_types.h>
#include <controller.h>
#include <errno.h>

int TransportRx_Process(struct HostTransport_Handle *hostHandle,
                        struct Controller_Handle *controllerHandle,
                        struct Adxl345_Handle *sensorHandle, uint8_t *buffer,
                        const uint32_t *length) {
  if (NULL == buffer || NULL == length)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
    // get firmware version
  case Transport_HeaderId_Rx_GetFirmwareVersion: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetFirmwareVersion) ==
        *length) {

      struct TransportFrame response = {
          .header.id = Transport_HeaderId_Tx_FirmwareVersion,
          .asTxFrame.asFirmwareVersion.major =
              controllerHandle->deviceVersionMajor,
          .asTxFrame.asFirmwareVersion.minor =
              controllerHandle->deviceVersionMinor,
          .asTxFrame.asFirmwareVersion.patch =
              controllerHandle->deviceVersionPatch};

      // send version
      while (HostTransport_Status_Busy ==
             hostHandle->transmit((uint8_t *)&response,
                                  SIZEOF_HEADER_INCL_PAYLOAD(
                                      response.asTxFrame.asFirmwareVersion)))
        ;
      return 0;
    }
  } break;
    // get ODR
  case Transport_HeaderId_Rx_GetOutputDataRate: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetOutputDataRate) ==
        *length) {
      enum Adxl345Flags_BwRate_Rate rate;
      Adxl345_getOutputDataRate(sensorHandle, &rate);

      struct TransportFrame response = {
          .header.id = Transport_HeaderId_Tx_OutputDataRate};
      response.asTxFrame.asOutputDataRate.rate =
          (enum TransportRx_SetOutputDataRate_Rate)rate;
      // send ODR
      while (HostTransport_Status_Busy ==
             hostHandle->transmit((uint8_t *)&response,
                                  SIZEOF_HEADER_INCL_PAYLOAD(
                                      response.asTxFrame.asOutputDataRate)))
        ;
      return 0;
    }
  } break;
    // set ODR
  case Transport_HeaderId_Rx_SetOutputDataRate:
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetOutputDataRate) ==
        *length) {
      return Adxl345_setOutputDataRate(
          sensorHandle, request->asRxFrame.asSetOutputDataRate.rate);
    }
    break;

    // get range
  case Transport_HeaderId_Rx_GetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetRange) == *length) {
      enum Adxl345Flags_DataFormat_Range range;
      Adxl345_getRange(sensorHandle, &range);

      struct TransportFrame response = {.header.id =
                                            Transport_HeaderId_Tx_Range};
      response.asTxFrame.asRange.range = (enum TransportRx_SetRange_Range)range;

      // send range
      while (HostTransport_Status_Busy ==
             hostHandle->transmit(
                 (uint8_t *)&response,
                 SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asRange)))
        ;
      return 0;
    }
  } break;

    // set range
  case Transport_HeaderId_Rx_SetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetRange) == *length) {
      return Adxl345_setRange(sensorHandle,
                              request->asRxFrame.asSetRange.range);
    }
  } break;

    // get scale
  case Transport_HeaderId_Rx_GetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetScale) == *length) {
      enum Adxl345Flags_DataFormat_FullResBit scale;
      Adxl345_getScale(sensorHandle, &scale);

      struct TransportFrame response = {.header.id =
                                            Transport_HeaderId_Tx_Scale};
      response.asTxFrame.asScale.scale = (enum TransportRx_SetScale_Scale)scale;

      // send scale
      while (HostTransport_Status_Busy ==
             hostHandle->transmit(
                 (uint8_t *)&response,
                 SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asScale)))
        ;
      return 0;
    }
  } break;

    // set scale
  case Transport_HeaderId_Rx_SetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetScale) == *length) {
      return Adxl345_setScale(sensorHandle,
                              request->asRxFrame.asSetScale.scale);
    }
  } break;

  // get device setup
  case Transport_HeaderId_Rx_GetDeviceSetup: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetDeviceSetup) ==
        *length) {
      enum Adxl345Flags_BwRate_Rate rate;
      enum Adxl345Flags_DataFormat_Range range;
      enum Adxl345Flags_DataFormat_FullResBit scale;
      Adxl345_getOutputDataRate(sensorHandle, &rate);
      Adxl345_getRange(sensorHandle, &range);
      Adxl345_getScale(sensorHandle, &scale);

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
             hostHandle->transmit(
                 (uint8_t *)&response,
                 SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asDeviceSetup)))
        ;
      return 0;
    }
  } break;

    // device reboot requested
  case Transport_HeaderId_Rx_DeviceReboot: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_DeviceReboot) ==
        *length) {
      controllerHandle->deviceRequestReboot();
      return 0;
    }
  } break;

    // sapling start requested
  case Transport_HeaderId_Rx_SamplingStart: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStart) ==
        *length) {
      controllerHandle->samplingStart(
          controllerHandle->samplingHandle,
          request->asRxFrame.asSamplingStart.max_samples_count);
      return 0;
    }
  } break;

    // sampling stop requested
  case Transport_HeaderId_Rx_SamplingStop: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStop) ==
        *length) {
      controllerHandle->samplingStop(controllerHandle->samplingHandle);
      return 0;
    }
  } break;

  default:
    break;
  }

  return -EINVAL;
}
