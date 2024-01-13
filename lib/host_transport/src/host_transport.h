/**
 * \file host_transport_types.h
 */

#pragma once
#include <inttypes.h>
#include <ringbuffer.h>

/**
 * Status returned by HostTransport_Handle.transmit(uint8_t *, uint16_t);
 */
enum HostTransport_Status {
  HostTransport_Status_Ok = 0,
  HostTransport_Status_Busy,
  HostTransport_Status_Again,
  HostTransport_Status_BufferOverflow,
  HostTransport_Status_Fail,
  HostTransport_Status_Undefined
};

struct HostTransport_FromHostApi {
  int (*const doTakeReceivedPacketImpl)(
      const uint8_t *); ///< Context: CDC_Receive_FS(uint8_t* , uint32_t *)
};

struct HostTransport_ToHostApi {
  /**
   * Buffer used to transmit chunks of acceleration data.
   *
   * This buffer is modified and then provided to the underlying transmit
   * implementation.
   * As long the transmission is ongoing, namely USB TX-busy flag is set,
   * this buffer must not be modified.
   * The buffer must only be utilized by TransportTx_TxAccelerationBuffer(
   * struct HostTransport_Handle *, const struct Transport_Acceleration *,
   * uint8_t, uint16_t).
   *
   * \see HostTransport_ToHostApi.ringbuffer
   *
   * Context: main()
   *
   * @{
   */
  uint8_t *txBuffer;
  const uint16_t txBufferSize;
  /// @}

  /**
   * Circular buffer for buffering outgoing acceleration packets.
   *
   * This buffer is used to pile up acceleration chunks when USB is busy and
   * transmission has to be postponed.
   * The buffer must only be utilized by TransportTx_TxAccelerationBuffer(
   * struct HostTransport_Handle *, const struct Transport_Acceleration *,
   * uint8_t, uint16_t).
   *
   * \see HostTransport_ToHostApi.txBuffer
   *
   * Context: main()
   */
  struct Ringbuffer ringbuffer;

  /**
   * Buffer performance indicator representing the maximum buffer level since
   * last sampling-start.
   */
  uint16_t ringbufferMaxItemsUtilization;

  enum HostTransport_Status (*const doTransmitImpl)(
      uint8_t *, uint16_t); ///< Context: main() and interrupts
};

/**
 * Host communication handle.
 */
struct HostTransport_Handle {
  struct HostTransport_FromHostApi fromHost;
  struct HostTransport_ToHostApi toHost;
};
