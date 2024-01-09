/**
 * \file host_transport_types.h
 */

#pragma once
#include <inttypes.h>

/**
 * Status returned by HostTransport_Handle.transmit(uint8_t *, uint16_t);
 */
enum HostTransport_Status {
  HostTransport_Status_Ok = 0,
  HostTransport_Status_Busy,
  HostTransport_Status_Fail,
  HostTransport_Status_Undefined
};

/**
 * Host communication handle.
 */
struct HostTransport_Handle {
  enum HostTransport_Status (*transmit)(uint8_t *,
                                        uint16_t); ///< host communication pimpl
};
