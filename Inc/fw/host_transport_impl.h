/**
 * \file host_transport_impl.h
 *
 * Hardware specific transport to the IN USB endpoint of the host.
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

enum HostTransport_Status;

enum HostTransport_Status HostTransportImpl_doTransmitImpl(uint8_t *buffer,
                                                           uint16_t len);
bool HostTransportImpl_isTransmitBusyImpl();

int HostTransportImpl_onTakeReceivedImpl(const uint8_t *buffer);
