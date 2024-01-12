/**
 * \file to_host_transport.c
 *
 * Implementation for transporting data from controller to host.
 */

#include "to_host_transport.h"
#include "host_transport.h"
#include "host_transport_types.h"
#include <errno.h>

/**
 * Transmits data to the IN endpoint of host.
 *
 * @param handle underlying pimpl
 * @param buffer data to transmit
 * @param len data length
 * @return transmission status
 */
static enum HostTransport_Status
TransportTx_transmit(struct HostTransport_Handle *handle, uint8_t *buffer,
                     uint16_t len) {
  return handle->toHost.doTransmitImpl(buffer, len);
}

/**
 * Transmits data to the IN endpoint of host.
 *
 * @param handle underlying pimpl
 * @param buffer data to transmit
 * @param len data length
 * @return transmission status
 */

/**
 * Transmits or buffers acceleration data blocks until next invocation.
 *
 * This implementation does not insist on completed transmission but buffers
 * instead.
 * The USB host will poll the usb client's IN endpoint about every 1ms
 * or lesser.
 * This is even slower on weak hardware such as raspberry pi or similar.
 *
 * @param handle
 * @param buffer the acceleration data block to transmit
 * @param len
 * @return
 *   - HostTransport_Status_Again if data is still buffered,
 *   - any other value of HostTransport_Status otherwise
 */
enum HostTransport_Status TransportTx_transmitAccelerationBuffered(
    struct HostTransport_Handle *handle,
    struct TransportFrame *accelerationsChunk, uint16_t dataCount) {
  // todo: replace naive forwarding with a buffered approach
  //   - for number of defined retries (i.e. 3x) do
  //     - if USB status is not busy
  //       1. if ringbuffer is not empty
  //          1.a pop data from ringbuffer to UserTxBufferFS
  //       2. if ringbuffer is empty
  //         2.a store new data directly in UserTxBufferFS
  //       3. transmit data from UserTxBufferFS
  //       4. reduce the retry counter by one
  //   done
  //   - if new data was not stored anywhere store in UserTxBufferFS if in ring
  //   buffer is empty,
  //     otherwise in ringbuffer or return HostTransport_Status_BufferOverflow
  //     accordingly

  // todo: document that the buffer UserTxBufferFS must remain untouched
  //    (only manipulated by this implementation)

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  while (HostTransport_Status_Busy ==
         handle->toHost.doTransmitImpl(
             (uint8_t *)accelerationsChunk,
             dataCount * (SIZEOF_HEADER_INCL_PAYLOAD(
                             struct TransportTx_Acceleration)))) {
  }

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
  return HostTransport_Status_Ok;
}

void TransportTx_TxSamplingSetup(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_DeviceSetup;

  uint8_t odr = {0};
  uint8_t scale = {0};
  uint8_t range = {0};

  handle->toHost.doGetSensorOutputDataRateImpl(&odr);
  handle->toHost.doGetSensorScaleImpl(&scale);
  handle->toHost.doGetSensorRangeImpl(&range);

  data.asTxFrame.asDeviceSetup.outputDataRate = odr;
  data.asTxFrame.asDeviceSetup.scale = scale;
  data.asTxFrame.asDeviceSetup.range = range;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asDeviceSetup))) {
  }
}

void TransportTx_TxScale(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Scale;
  uint8_t scale = {0};
  handle->toHost.doGetSensorScaleImpl(&scale);
  data.asTxFrame.asScale.scale = scale;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asScale))) {
  }
}

void TransportTx_TxRange(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Range;
  uint8_t range = {0};
  handle->toHost.doGetSensorRangeImpl(&range);
  data.asTxFrame.asRange.range = range;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asRange))) {
  }
}

void TransportTx_TxOutputDataRate(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_OutputDataRate;
  uint8_t odr = {0};
  handle->toHost.doGetSensorOutputDataRateImpl(&odr);
  data.asTxFrame.asOutputDataRate.rate = odr;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asOutputDataRate))) {
  }
}

void TransportTx_TxUptime(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Uptime;
  data.asTxFrame.asUptime.elapsedMs = handle->toHost.doGetUptimeMsImpl();
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asUptime))) {
  }
}

void TransportTx_TxError(struct HostTransport_Handle *handle,
                         enum TransportTx_ErrorCode code) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Error;
  data.asTxFrame.asError.code = code;
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asError))) {
  }
}

void TransportTx_TxFirmwareVersion(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_FirmwareVersion;
  data.asTxFrame.asFirmwareVersion.major =
      handle->toHost.controllerVersionMajor;
  data.asTxFrame.asFirmwareVersion.minor =
      handle->toHost.controllerVersionMinor;
  data.asTxFrame.asFirmwareVersion.patch =
      handle->toHost.controllerVersionPatch;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asFirmwareVersion))) {
  }
}

void TransportTx_TxSamplingStarted(struct HostTransport_Handle *handle,
                                   uint16_t max_samples) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_SamplingStarted;
  data.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingStarted))) {
  }
}

void TransportTx_TxSamplingFinished(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingFinished};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingFinished))) {
  }
}

void TransportTx_TxSamplingStopped(struct HostTransport_Handle *handle) {
  TransportTx_TxSamplingSetup(handle);

  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingStopped};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingStopped))) {
  }
}

void TransportTx_TxSamplingAborted(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingAborted};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingAborted))) {
  }
}

void TransportTx_TxFifoOverflow(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_FifoOverflow};
  while (HostTransport_Status_Busy ==
         TransportTx_transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asFifoOverflow))) {
  }
}

int TransportTx_TxAccelerationBuffer(
    struct HostTransport_Handle *handle,
    const struct Transport_Acceleration *data,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint8_t count, uint16_t firstIndex) {
  if (TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER < count || NULL == data) {
    return -EINVAL;
  }

  uint8_t
      rawBuffer[TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER *
                (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration))];
  struct TransportFrame *accelerationsChunk =
      (struct TransportFrame *)rawBuffer;

  for (uint8_t idx = 0; idx < count; idx++) {
    accelerationsChunk[idx].asTxFrame.asAcceleration.index = firstIndex++;
    accelerationsChunk[idx].asTxFrame.asAcceleration.values.x = data[idx].x;
    accelerationsChunk[idx].asTxFrame.asAcceleration.values.y = data[idx].y;
    accelerationsChunk[idx].asTxFrame.asAcceleration.values.z = data[idx].z;
    accelerationsChunk[idx].header.id = Transport_HeaderId_Tx_Acceleration;
  }

  return TransportTx_transmitAccelerationBuffered(handle, accelerationsChunk,
                                                  count);
}
