/**
 * \file host_transport_impl.h
 *
 * API Controller to IN USB endpoint (host) transport.
 */

#pragma once
#include <inttypes.h>

enum HostTransport_Status;

#define HOSTTRANSPORT_DECLARE_INITIALIZER                                      \
  {                                                                            \
    .transmit = HostTransportImpl_transmit,                                    \
    .onPacketReceived = HostTransportImpl_onPacketReceived                     \
  }

enum HostTransport_Status HostTransportImpl_transmit(uint8_t *buffer,
                                                     uint16_t len);
int HostTransportImpl_onPacketReceived(uint8_t *buffer);
