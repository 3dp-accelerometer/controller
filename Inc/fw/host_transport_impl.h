/**
 * \file host_transport_impl.h
 *
 * API Controller to IN USB endpoint (host) transport.
 */

#pragma once
#include <inttypes.h>

struct HostTransport_Handle;
enum HostTransport_Status;

#define HOSTTRANSPORT_DECLARE_HANDLE(HANDLE_NAME)                              \
  struct HostTransport_Handle HANDLE_NAME = {.transmit =                       \
                                                 HostTransportImpl_transmit}

enum HostTransport_Status HostTransportImpl_transmit(uint8_t *buffer,
                                                     uint16_t len);
