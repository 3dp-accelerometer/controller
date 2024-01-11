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
    .fromHost = {.doTakeReceivedPacketImpl =                                   \
                     HostTransportImpl_onTakeReceivedImpl},                    \
    .toHost = {                                                                \
      .doTransmitImpl = HostTransportImpl_doTransmitImpl,                      \
      .controllerVersionMajor = VERSION_MAJOR,                                 \
      .controllerVersionMinor = VERSION_MINOR,                                 \
      .controllerVersionPatch = VERSION_PATCH,                                 \
      .doGetSensorOutputDataRateImpl =                                         \
          ControllerImpl_sensor_Adxl345_doGetOutputDataRateImpl,               \
      .doGetSensorScaleImpl = ControllerImpl_sensor_Adxl345_doGetScaleImpl,    \
      .doGetSensorRangeImpl = ControllerImpl_sensor_Adxl345_doGetRangeImpl     \
    }                                                                          \
  }

enum HostTransport_Status HostTransportImpl_doTransmitImpl(uint8_t *buffer,
                                                           uint16_t len);
int HostTransportImpl_onTakeReceivedImpl(uint8_t *buffer);
