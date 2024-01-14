/**
 * \file to_host_transport.c
 *
 * Implementation for transporting data from controller to host.
 */

#include "to_host_transport.h"
#include "fw/debug.h"
#include "host_transport.h"
#include "host_transport_types.h"
#include <errno.h>

static volatile bool
isTransmitComplete(const struct HostTransport_ToHostApi *toHostApi) {
  return !toHostApi->isTransmitBusyImpl();
}

/**
 * Transmits data to the IN endpoint of host.
 *
 * The transmission blocks this function from returning until the transmission
 * has completely finished.
 *
 * @param handle underlying pimpl
 * @param buffer data to transmit
 * @param len data length
 * @return transmission status
 */
uint8_t transmit(struct HostTransport_Handle *handle, uint8_t *buffer,
                 uint16_t len) {
  return handle->toHost.doTransmitImpl(buffer, len);
}

/**
 * Transmits or buffers acceleration data blocks to the IN endpoint of host or
 * in ringbuffer.
 *
 * This implementation either transmits or buffers data but does not insist on
 * completed transmission.
 * The USB host will poll the usb client's IN endpoint about every 1ms
 * or lesser.
 * This is even slower on weak hardware such as Raspberry Pi or similar.
 *
 * Notes:
 *   - UserTxBufferFS must remain untouched by any other function
 *   - UserTxBufferFS is only modified by this implementation
 *
 * Findings:
 *   - on RPi 4B the Pyserial performance is a bottleneck when receiving with
 *     ODR1600 or higher
 *   - hiccups of about 20ms have been observed where Pyserial did not consume
 *     bytes from the serial device;
 *     this in turn blocks the device when sending (USB device is busy on TX)
 *   - USB host or kernel driver are not expected to be subjects of performance
 *     issues (at the time of writing)
 *   - before tinkering with the Python performance, buffering on the controller
 *     is a reasonable mitigation strategy
 *   - Example: a buffer of about 1s at highest sample rate cold be as follows
 *     3200kS/s * (1+2+6)B * 1s = 28800B
 *
 * @param handle
 * @param accelerationsChunk the acceleration data block to transmit, NULL to
 * send pending data; Note: the buffer must be packed and contain only data of
 * type: Transport_Header + TransportTx_Acceleration.
 * @param dataCount amount of items in buffer (Transport_Header +
 * TransportTx_Acceleration)
 * @return
 *   - -ENOMEM if ringbuffer is exhausted
 *   - -ENODATA if all data is sent
 *   - -EAGAIN if a subsequent call would send pending data
 */
static int
transmitAccelerationBuffered(struct HostTransport_Handle *handle,
                             struct TransportFrame *accelerationsChunk,
                             uint16_t dataCount) {

  static uint16_t ringbufferUtilization = {0};

  // copy whole acceleration data chunk to ringbuffer

  for (uint16_t idx = 0; idx < dataCount; idx++) {
    if (Ringbuffer_isFull(&handle->toHost.ringbuffer)) {
      return -ENOMEM;
    }
    Ringbuffer_put(&handle->toHost.ringbuffer, &accelerationsChunk[idx]);
    ringbufferUtilization++;
  }

  // update ringbuffer utilization
  handle->toHost.ringbufferMaxItemsUtilization =
      (ringbufferUtilization > handle->toHost.ringbufferMaxItemsUtilization)
          ? ringbufferUtilization
          : handle->toHost.ringbufferMaxItemsUtilization;

  if (Ringbuffer_isEmpty(&handle->toHost.ringbuffer)) {
    return -ENODATA;
  }

  if (!isTransmitComplete(&handle->toHost)) {
    return -EAGAIN;
  }

  // pop data from buffer and store to TX-buffer

  // todo: in the meanwhile the xmission could be ongoing again
  //   reasons:
  //   - fault handler: neglect-able bcs. those usually run into endless loops
  //   - response upon user request:
  //     - controller shall not accept host commands while sampling (except
  //     reboot command)
  //     - alternatively introduce locks

  const uint8_t sizeofItem = {
      SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration)};

  static_assert(__builtin_types_compatible_p(typeof(uint8_t *),
                                             typeof(handle->toHost.txBuffer)),
                "ERROR: pointer arithmetic will fail if types are not same. "
                "Required buffer type: uint8_t.");
  uint8_t *byteBuffer = handle->toHost.txBuffer;

  uint8_t poppedItemsCount = 0;
  while ((poppedItemsCount * sizeofItem < handle->toHost.txBufferSize) &&
         (!Ringbuffer_isEmpty(&handle->toHost.ringbuffer))) {
    struct TransportTx_Acceleration *nextInTxBuffer =
        (struct TransportTx_Acceleration
             *)&byteBuffer[(uint8_t)(sizeofItem * poppedItemsCount)];
    Ringbuffer_take(&handle->toHost.ringbuffer, nextInTxBuffer);
    ringbufferUtilization--;
    poppedItemsCount++;
  }

  // transmit tx buffer

  if (poppedItemsCount > 0) {
    USER_DEBUG0_HIGH; // mark the start of transmission
    handle->toHost.doTransmitImpl(handle->toHost.txBuffer,
                                  poppedItemsCount * sizeofItem);
  }

  return -EAGAIN;
}

void TransportTx_TxSamplingSetup(
    struct HostTransport_Handle *handle,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint8_t sensorOdr, uint8_t sensorScale, uint8_t sensorRange) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_DeviceSetup;

  data.asTxFrame.asDeviceSetup.outputDataRate = sensorOdr;
  data.asTxFrame.asDeviceSetup.scale = sensorScale;
  data.asTxFrame.asDeviceSetup.range = sensorRange;

  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asDeviceSetup))) {
  }
}

void TransportTx_TxScale(struct HostTransport_Handle *handle,
                         uint8_t sensorScale) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Scale;
  data.asTxFrame.asScale.scale = sensorScale;

  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asScale))) {
  }
}

void TransportTx_TxRange(struct HostTransport_Handle *handle,
                         uint8_t sensorRange) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Range;
  data.asTxFrame.asRange.range = sensorRange;

  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asRange))) {
  }
}

void TransportTx_TxOutputDataRate(struct HostTransport_Handle *handle,
                                  uint8_t sensorOdr) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_OutputDataRate;
  data.asTxFrame.asOutputDataRate.rate = sensorOdr;

  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asOutputDataRate))) {
  }
}

void TransportTx_TxFirmwareVersion(
    struct HostTransport_Handle *handle,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint8_t major, uint8_t minor, uint8_t patch) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_FirmwareVersion;
  data.asTxFrame.asFirmwareVersion.major = major;
  data.asTxFrame.asFirmwareVersion.minor = minor;
  data.asTxFrame.asFirmwareVersion.patch = patch;

  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asFirmwareVersion))) {
  }
}

void TransportTx_TxSamplingStarted(struct HostTransport_Handle *handle,
                                   uint16_t max_samples) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_SamplingStarted;
  data.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingStarted))) {
  }
}

void TransportTx_TxSamplingFinished(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingFinished};
  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingFinished))) {
  }
}

void TransportTx_TxSamplingStopped(struct HostTransport_Handle *handle,
                                   uint8_t sensorOdr, uint8_t sensorScale,
                                   uint8_t sensorRange) {
  TransportTx_TxSamplingSetup(handle, sensorOdr, sensorScale, sensorRange);

  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingStopped};
  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingStopped))) {
  }
}

void TransportTx_TxSamplingAborted(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_SamplingAborted};
  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asSamplingAborted))) {
  }
}

void TransportTx_TxFifoOverflow(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_FifoOverflow};
  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asFifoOverflow))) {
  }
}

void TransportTx_TxBufferOverflow(struct HostTransport_Handle *handle) {
  struct TransportFrame data = {.header.id =
                                    Transport_HeaderId_Tx_BufferOverflow};
  while (
      HostTransport_Status_Busy ==
      transmit(handle, (uint8_t *)&data,
               SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asBufferOverflow))) {
  }
}

int TransportTx_TxAccelerationBuffer(
    struct HostTransport_Handle *handle,
    const struct Transport_Acceleration *data,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint8_t count, uint16_t firstIndex) {

  if (TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES < count || NULL == data) {
    return transmitAccelerationBuffered(handle, NULL, 0);
  }

  // packed buffer containing: Transport_Header + TransportTx_Acceleration
  uint8_t
      byteBuffer[TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES *
                 SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration)];

  // packed copy of samples inclusive  sequence numbering
  for (uint8_t idx = 0; idx < count; idx++) {
    uint8_t sizeofItem = {
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration)};
    struct TransportFrame *frame = {
        (struct TransportFrame *)&byteBuffer[(uint8_t)(idx * sizeofItem)]};
    frame->asTxFrame.asAcceleration.index = firstIndex++;
    frame->asTxFrame.asAcceleration.values.x = data[idx].x;
    frame->asTxFrame.asAcceleration.values.y = data[idx].y;
    frame->asTxFrame.asAcceleration.values.z = data[idx].z;
    frame->header.id = Transport_HeaderId_Tx_Acceleration;
  }
  return transmitAccelerationBuffered(
      handle, (struct TransportFrame *)&byteBuffer[0], count);
}

void TransportTx_TxUptime(struct HostTransport_Handle *handle,
                          uint32_t uptimeMs) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Uptime;
  data.asTxFrame.asUptime.elapsedMs = uptimeMs;
  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asUptime))) {
  }
}

void TransportTx_TxFault(struct HostTransport_Handle *handle,
                         enum TransportTx_FaultCode code) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_Fault;
  data.asTxFrame.asFault.code = code;
  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asFault))) {
  }
}

void TransportTx_BufferStatus(
    struct HostTransport_Handle *handle,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint16_t sizeBytes, uint16_t capacity, uint16_t maxItemsCount) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_BufferStatus;
  data.asTxFrame.asBufferStatus.sizeBytes = sizeBytes;
  data.asTxFrame.asBufferStatus.capacity = capacity;
  data.asTxFrame.asBufferStatus.maxItemsCount = maxItemsCount;
  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asBufferStatus))) {
  }
}
