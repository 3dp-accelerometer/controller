/**
 * \file from_host_transport.c
 *
 * Implementation for processing data from host to controller.
 */

#include "host_transport.h"
#include "host_transport_types.h"
#include <errno.h>

int TransportRx_Process(struct HostTransport_Handle *handle, uint8_t *buffer,
                        uint16_t length) {
  if (NULL == buffer || length > 65335)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
    // get firmware version
  case Transport_HeaderId_Rx_GetFirmwareVersion: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetFirmwareVersion) ==
        length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;
    // get ODR
  case Transport_HeaderId_Rx_GetOutputDataRate: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetOutputDataRate) ==
        length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;
    // set ODR
  case Transport_HeaderId_Rx_SetOutputDataRate:
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetOutputDataRate) ==
        length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
    break;

    // get range
  case Transport_HeaderId_Rx_GetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetRange) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // set range
  case Transport_HeaderId_Rx_SetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetRange) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // get scale
  case Transport_HeaderId_Rx_GetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetScale) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // set scale
  case Transport_HeaderId_Rx_SetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetScale) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

  // get device setup
  case Transport_HeaderId_Rx_GetDeviceSetup: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetDeviceSetup) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // device reboot requested
  case Transport_HeaderId_Rx_DeviceReboot: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_DeviceReboot) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // sapling start requested
  case Transport_HeaderId_Rx_SamplingStart: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStart) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

    // sampling stop requested
  case Transport_HeaderId_Rx_SamplingStop: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStop) == length)
      return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  } break;

  default:
    return -EINVAL;
  }

  return -EINVAL;
}
