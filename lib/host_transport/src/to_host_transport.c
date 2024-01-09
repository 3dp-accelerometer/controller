/**
 * \file to_host_transport.c
 *
 * Implementation for transporting data from controller to host.
 */

#include "host_transport.h"
#include "host_transport_types.h"
#include <adxl345.h>
#include <adxl345_flags.h>
#include <adxl345_transport_types.h>
#include <controller.h>
#include <errno.h>

void TransportTx_SamplingSetup(struct HostTransport_Handle *hostHandle,
                               struct Adxl345_Handle *sensorHandle) {
  struct TransportFrame tx;

  enum Adxl345Flags_BwRate_Rate rate;
  enum Adxl345Flags_DataFormat_Range range;
  enum Adxl345Flags_DataFormat_FullResBit scale;

  Adxl345_getOutputDataRate(sensorHandle, &rate);
  Adxl345_getScale(sensorHandle, &scale);
  Adxl345_getRange(sensorHandle, &range);

  tx.header.id = Transport_HeaderId_Tx_DeviceSetup;
  tx.asTxFrame.asDeviceSetup.outputDataRate = rate;
  tx.asTxFrame.asDeviceSetup.range = range;
  tx.asTxFrame.asDeviceSetup.scale = scale;

  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx, SIZEOF_HEADER_INCL_PAYLOAD(
                                                  tx.asTxFrame.asDeviceSetup)))
    ;
}

void TransportTx_FirmwareVersion(struct HostTransport_Handle *hostHandle,
                                 struct Controller_Handle *controllerHandle) {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_FirmwareVersion;
  tx.asTxFrame.asFirmwareVersion.major = controllerHandle->swVersionMajor;
  tx.asTxFrame.asFirmwareVersion.minor = controllerHandle->swVersionMinor;
  tx.asTxFrame.asFirmwareVersion.patch = controllerHandle->swVersionPatch;

  while (HostTransport_Status_Busy ==
         hostHandle->transmit(
             (uint8_t *)&tx,
             SIZEOF_HEADER_INCL_PAYLOAD(tx.asTxFrame.asFirmwareVersion)))
    ;
}

void TransportTx_SamplingStarted(struct HostTransport_Handle *hostHandle,
                                 uint16_t max_samples) {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_SamplingStarted;
  tx.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingStarted)))
    ;
}

void TransportTx_SamplingFinished(struct HostTransport_Handle *hostHandle) {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingFinished};
  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingFinished)))
    ;
}

void TransportTx_SamplingStopped(struct HostTransport_Handle *hostHandle,
                                 struct Adxl345_Handle *sensorHandle) {
  TransportTx_SamplingSetup(hostHandle, sensorHandle);

  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingStopped};
  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingStopped)))
    ;
}

void TransportTx_SamplingAborted(struct HostTransport_Handle *hostHandle) {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingAborted};
  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asSamplingAborted)))
    ;
}

void TransportTx_FifoOverflow(struct HostTransport_Handle *hostHandle) {
  struct TransportFrame tx = {.header.id = Transport_HeaderId_Tx_FifoOverflow};
  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)&tx,
                              sizeof(struct Transport_Header) +
                                  sizeof(tx.asTxFrame.asFifoOverflow)))
    ;
}

int TransportTx_AccelerationBuffer(struct HostTransport_Handle *hostHandle,
                                   struct Adxl345Transport_Acceleration *data,
                                   uint8_t count, uint16_t startIndex) {
  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  struct TransportFrame acc[ADXL345_WATERMARK_LEVEL] = {};

  if (ADXL345_WATERMARK_LEVEL < count || NULL == data) {
    return -EINVAL;
  }

  // todo
  assert(sizeof(struct TransportFrame) ==
         (sizeof(struct TransportTx_Acceleration) +
          sizeof(struct Transport_Header)));

  for (uint8_t idx = 0; idx < count; idx++) {
    acc[idx].asTxFrame.asAcceleration.index = startIndex++;
    acc[idx].asTxFrame.asAcceleration.x = data[idx].x;
    acc[idx].asTxFrame.asAcceleration.y = data[idx].y;
    acc[idx].asTxFrame.asAcceleration.z = data[idx].z;
    acc[idx].header.id = Transport_HeaderId_Tx_Acceleration;
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

  while (HostTransport_Status_Busy ==
         hostHandle->transmit((uint8_t *)acc,
                              count * sizeof(struct TransportFrame)))
    ;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
  return 0;
}
