/**
 * \file host_transport_impl.h
 *
 * API Controller to IN USB endpoint (host) transport.
 */

#pragma once
#include <inttypes.h>

struct HostTransport_Handle;
enum HostTransport_Status;

enum HostTransport_Status HostTransportImpl_transmit(uint8_t *buffer, uint16_t len);

int HostTransportImpl_initHandle(struct HostTransport_Handle *handle);
