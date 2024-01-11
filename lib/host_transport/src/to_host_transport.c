/**
 * \file to_host_transport.c
 *
 * Implementation for transporting data from controller to host.
 */

#include "to_host_transport.h"
#include "host_transport.h"
#include "host_transport_types.h"
#include <errno.h>

enum HostTransport_Status
TransportTx_transmit(struct HostTransport_Handle *handle, uint8_t *buffer,
                     uint16_t len) {
  return handle->toHost.doTransmitImpl(buffer, len);
}

void TransportTx_SamplingSetup(struct HostTransport_Handle *handle) {
  struct TransportFrame tx;

  tx.header.id = Transport_HeaderId_Tx_DeviceSetup;

  uint8_t odr = {0};
  uint8_t scale = {0};
  uint8_t range = {0};

  handle->toHost.doGetSensorOutputDataRateImpl(&odr);
  handle->toHost.doGetSensorScaleImpl(&scale);
  handle->toHost.doGetSensorRangeImpl(&range);

  tx.asTxFrame.asDeviceSetup.outputDataRate = odr;
  tx.asTxFrame.asDeviceSetup.scale = scale;
  tx.asTxFrame.asDeviceSetup.range = range;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&tx,
             SIZEOF_HEADER_INCL_PAYLOAD(tx.asTxFrame.asDeviceSetup)))
    ;
}

void TransportTx_FirmwareVersion(struct HostTransport_Handle *handle) {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_FirmwareVersion;
  tx.asTxFrame.asFirmwareVersion.major = handle->toHost.controllerVersionMajor;
  tx.asTxFrame.asFirmwareVersion.minor = handle->toHost.controllerVersionMinor;
  tx.asTxFrame.asFirmwareVersion.patch = handle->toHost.controllerVersionPatch;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&tx,
             SIZEOF_HEADER_INCL_PAYLOAD(tx.asTxFrame.asFirmwareVersion)))
    ;
}

void TransportTx_SamplingStarted(struct HostTransport_Handle *handle,
                                 uint16_t max_samples) {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_SamplingStarted;
  tx.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(handle, (uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingStarted)))
    ;
}

void TransportTx_SamplingFinished(struct HostTransport_Handle *handle) {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingFinished};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(handle, (uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingFinished)))
    ;
}

void TransportTx_SamplingStopped(struct HostTransport_Handle *handle) {
  TransportTx_SamplingSetup(handle);

  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingStopped};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(handle, (uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingStopped)))
    ;
}

void TransportTx_SamplingAborted(struct HostTransport_Handle *handle) {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingAborted};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(handle, (uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingAborted)))
    ;
}

void TransportTx_FifoOverflow(struct HostTransport_Handle *handle) {
  struct TransportFrame tx = {.header.id = Transport_HeaderId_Tx_FifoOverflow};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(handle, (uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asFifoOverflow)))
    ;
}

int TransportTx_AccelerationBuffer(struct HostTransport_Handle *handle,
                                   const struct Transport_Acceleration *data,
                                   uint8_t count, uint16_t startIndex) {
  if (TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER < count || NULL == data) {
    return -EINVAL;
  }

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);

  uint8_t rawBuffer[TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER *
                    (sizeof(struct Transport_Header) +
                     sizeof(struct TransportTx_Acceleration))];
  struct TransportFrame *fameBuffer = (struct TransportFrame *)rawBuffer;

  for (uint8_t idx = 0; idx < count; idx++) {
    fameBuffer[idx].asTxFrame.asAcceleration.index = startIndex++;
    fameBuffer[idx].asTxFrame.asAcceleration.values.x = data[idx].x;
    fameBuffer[idx].asTxFrame.asAcceleration.values.y = data[idx].y;
    fameBuffer[idx].asTxFrame.asAcceleration.values.z = data[idx].z;
    fameBuffer[idx].header.id = Transport_HeaderId_Tx_Acceleration;
  }

  // todo: don't insist for completed transmission here. usb host (pc) will poll
  //   the usb client (us) for transactions about every 1ms or lesser. this is
  //   even more visible on weak hardware (i.e. raspberry pi).
  //   suggestion:
  //     1 apply Ringbuffer from <ringbuffer.h>
  //     2 store complete Adxl345TP_Acceleration buffer to ringbuffer
  //     3 try to send from ringbuffer: several items, as many possible but
  //     timeboxed 4 return ENODATA (nothing to transmit) or EBUSY (data
  //     pending) respectively 5 send remaining next time we are called 6 caller
  //     shall call us until ENODATA

  while (
      HostTransport_Status_Busy ==
      TransportTx_transmit(handle, (uint8_t *)rawBuffer,
                           count * (sizeof(struct Transport_Header) +
                                    sizeof(struct TransportTx_Acceleration)))) {
  }

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
  return 0;
}
