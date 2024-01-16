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
isTransmitBusy(const struct HostTransport_ToHostApi *toHostApi) {
  return toHostApi->isTransmitBusyImpl();
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
static uint8_t transmit(struct HostTransport_Handle *handle, uint8_t *buffer,
                        uint16_t len) {
  return handle->toHost.doTransmitImpl(buffer, len);
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

void TransportTx_TxSamplingStopped(struct HostTransport_Handle *handle) {

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

void TransportTx_TxTransmissionError(struct HostTransport_Handle *handle) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_TransmissionError;
  while (HostTransport_Status_Busy ==
         transmit(
             handle, (uint8_t *)&data,
             SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asTransmissionError))) {
  }
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

void TransportTx_TxBufferStatus(
    struct HostTransport_Handle *handle,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint16_t sizeBytes, uint16_t capacityTotal, uint16_t capacityUsedMax,
    uint16_t putCount, uint16_t takeCount, uint16_t largestTxChunkBytes) {
  struct TransportFrame data;
  data.header.id = Transport_HeaderId_Tx_BufferStatus;
  data.asTxFrame.asBufferStatus.sizeBytes = sizeBytes;
  data.asTxFrame.asBufferStatus.capacityTotal = capacityTotal;
  data.asTxFrame.asBufferStatus.capacityUsedMax = capacityUsedMax;
  data.asTxFrame.asBufferStatus.putCount = putCount;
  data.asTxFrame.asBufferStatus.takeCount = takeCount;
  data.asTxFrame.asBufferStatus.largestTxChunkBytes = largestTxChunkBytes;
  while (HostTransport_Status_Busy ==
         transmit(handle, (uint8_t *)&data,
                  SIZEOF_HEADER_INCL_PAYLOAD(data.asTxFrame.asBufferStatus))) {
  }
}

static int pushToRingbuffer(struct HostTransport_Handle *handle,
                            struct TransportFrame *accelerationsChunk,
                            uint16_t dataCount) {
  const uint16_t sizeofItem = {
      Ringbuffer_itemSizeBytes(&handle->toHost.ringbuffer)};

  // copy whole acceleration data chunk (if any) to ringbuffer
  for (uint16_t idx = 0; idx < dataCount; idx++) {
    if (Ringbuffer_isFull(&handle->toHost.ringbuffer)) {
      return -ENOMEM;
    }

    const struct TransportFrame *nextInChunk = (struct TransportFrame *)&(
        (uint8_t *)accelerationsChunk)[(uint16_t)(sizeofItem * idx)];

    Ringbuffer_put(&handle->toHost.ringbuffer, nextInChunk);
  }

  return 0;
}

static uint16_t popDataFromRingbuffer(struct HostTransport_Handle *handle,
                                      uint8_t *txBuffer,
                                      uint16_t txBufferSize) {

  const uint16_t sizeofItem = {
      Ringbuffer_itemSizeBytes(&handle->toHost.ringbuffer)};
  uint16_t poppedItemsCount = 0;

  // pop data from buffer (if any) and store to TX-buffer

  // NOLINTNEXTLINE(altera-id-dependent-backward-branch)
  while ((!Ringbuffer_isEmpty(&handle->toHost.ringbuffer)) &&
         ((sizeofItem * (poppedItemsCount + 1)) < txBufferSize)) {

    struct TransportFrame *nextInTxBuffer = (struct TransportFrame *)&txBuffer[(
        uint16_t)(sizeofItem * poppedItemsCount)];

    Ringbuffer_take(&handle->toHost.ringbuffer, nextInTxBuffer);
    poppedItemsCount++;
  }

  return poppedItemsCount;
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
 *   - -EIO any other errors
 */
static int
transmitAccelerationBuffered(struct HostTransport_Handle *handle,
                             struct TransportFrame *accelerationsChunk,
                             uint16_t dataCount) {

  // store data to ringbuffer

  if (-ENOMEM == pushToRingbuffer(handle, accelerationsChunk, dataCount)) {
    return -ENOMEM;
  }

  // check if we should/can take data from ringbuffer

  if (isTransmitBusy(&handle->toHost)) {
    return -EAGAIN;
  }

  if (Ringbuffer_isEmpty(&handle->toHost.ringbuffer)) {
    return -ENODATA;
  }

  // pop data from buffer and store to TX-buffer

  static uint8_t byteBuffer[TRANSPORTTX_TRANSMIT_TX_DATA_CHUNK_BUFFER_BYTES] = {
      0};

  uint16_t poppedItemsCount = {popDataFromRingbuffer(
      handle, byteBuffer, TRANSPORTTX_TRANSMIT_TX_DATA_CHUNK_BUFFER_BYTES)};

  // transmit tx buffer

  const uint16_t sizeofItem = {
      Ringbuffer_itemSizeBytes(&handle->toHost.ringbuffer)};

  // todo: enforce that no transmission can be initiated from isTransmitBusy()
  //   until transmission doTransmitImpl()

  const uint16_t txBytes = {sizeofItem * poppedItemsCount};

  if (txBytes > handle->toHost.largestTxChunkBytes) {
    handle->toHost.largestTxChunkBytes = txBytes;
  }

  if (poppedItemsCount > 0) {
    if (HostTransport_Status_Fail ==
        handle->toHost.doTransmitImpl(byteBuffer, txBytes)) {
      return -EIO;
    }
  }

  return -EAGAIN;
}

int TransportTx_TxAccelerationBuffer(
    struct HostTransport_Handle *handle,
    const struct Transport_Acceleration *data,
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    uint8_t count, uint16_t firstIndex) {

  if (TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES < count) {
    return -EINVAL;
  }

  if (0 == count || NULL == data) {
    // transmit pending data
    return transmitAccelerationBuffered(handle, NULL, 0);
  }

  // packed buffer containing: Transport_Header + TransportTx_Acceleration
  uint8_t
      byteBuffer[TRANSPORTTX_TRANSMIT_ACCELERATION_BUFFER_BYTES *
                 SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration)];

  // pack a copy of samples inclusive sequence numbering
  for (uint8_t idx = 0; idx < count; idx++) {

    const uint8_t sizeofItem = {
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportTx_Acceleration)};

    struct TransportFrame *frame = {
        (struct TransportFrame *)&byteBuffer[(uint16_t)(idx * sizeofItem)]};

    frame->asTxFrame.asAcceleration.index = firstIndex++;
    frame->asTxFrame.asAcceleration.values.x = data[idx].x;
    frame->asTxFrame.asAcceleration.values.y = data[idx].y;
    frame->asTxFrame.asAcceleration.values.z = data[idx].z;
    frame->header.id = Transport_HeaderId_Tx_Acceleration;
  }
  return transmitAccelerationBuffered(
      handle, (struct TransportFrame *)byteBuffer, count);
}
