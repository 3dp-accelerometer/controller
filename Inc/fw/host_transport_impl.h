/**
 * \file host_transport_impl.h
 *
 * API Controller to IN USB endpoint (host) transport.
 */

#pragma once
#include "fw/version.h"
#include <inttypes.h>
enum HostTransport_Status;

#define HOSTTRANSPORT_DECLARE_INITIALIZER                                      \
  {                                                                            \
    .fromHost = {.onPacketReceived = HostTransportImpl_onPacketReceived},      \
    .toHost = {                                                                \
      .transmit = HostTransportImpl_transmit,                                  \
      .controllerVersionMajor = VERSION_MAJOR,                                 \
      .controllerVersionMinor = VERSION_MINOR,                                 \
      .controllerVersionPatch = VERSION_PATCH,                                 \
      .getSensorOutputDataRate =                                               \
          ControllerImpl_sensor_Adxl345_getOutputDataRate,                     \
      .getSensorScale = ControllerImpl_sensor_Adxl345_getScale,                \
      .getSensorRange = ControllerImpl_sensor_Adxl345_getRange                 \
    }                                                                          \
  }

enum HostTransport_Status HostTransportImpl_transmit(uint8_t *buffer,
                                                     uint16_t len);
int HostTransportImpl_onPacketReceived(uint8_t *buffer);
