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

struct HostTransport_FromHostApi {
  int (*onPacketReceived)(uint8_t *);
};

struct HostTransport_ToHostApi {
  enum HostTransport_Status (*transmit)(uint8_t *, uint16_t);

  uint8_t controllerVersionMajor;
  uint8_t controllerVersionMinor;
  uint8_t controllerVersionPatch;

  int (*getSensorOutputDataRate)(uint8_t *);
  int (*getSensorScale)(uint8_t *);
  int (*getSensorRange)(uint8_t *);
};

/**
 * Host communication handle.
 */
struct HostTransport_Handle {
  struct HostTransport_FromHostApi fromHost;
  struct HostTransport_ToHostApi toHost;
};
