/**
 * \file from_host_transport.c
 *
 * Implementation for processing data from host to controller.
 */

#include "host_transport.h"
#include "host_transport_types.h"
#include <errno.h>
#include <stdbool.h>

int TransportRx_Process(struct HostTransport_Handle *handle,
                        const uint8_t *buffer, uint16_t length) {
  if (NULL == buffer) {
    return -EINVAL;
  }

  bool sizeOk = {false};

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
    // NOLINTNEXTLINE(bugprone-branch-clone)
  case Transport_HeaderId_Rx_GetFirmwareVersion:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(
                 struct TransportRx_GetFirmwareVersion) == length;
    break;
  case Transport_HeaderId_Rx_GetOutputDataRate:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetOutputDataRate) ==
             length;
    break;
  case Transport_HeaderId_Rx_SetOutputDataRate:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetOutputDataRate) ==
             length;
    break;
  case Transport_HeaderId_Rx_GetRange:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetRange) == length;
    break;
  case Transport_HeaderId_Rx_SetRange:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetRange) == length;
    break;
  case Transport_HeaderId_Rx_GetScale:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetScale) == length;
    break;
  case Transport_HeaderId_Rx_SetScale:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetScale) == length;
    break;
  case Transport_HeaderId_Rx_GetDeviceSetup:
    sizeOk =
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetDeviceSetup) == length;
    break;
  case Transport_HeaderId_Rx_DeviceReboot:
    sizeOk =
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_DeviceReboot) == length;
    break;
  case Transport_HeaderId_Rx_SamplingStart:
    sizeOk =
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStart) == length;
    break;
  case Transport_HeaderId_Rx_SamplingStop:
    sizeOk =
        SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStop) == length;
    break;
  case Transport_HeaderId_Rx_GetUptime:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetUptime) == length;
    break;
  case Transport_HeaderId_Rx_GetBufferStatus:
    sizeOk = SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetBufferStatus) ==
             length;
    break;

  default:
    return -EINVAL;
  }

  if (sizeOk) {
    return handle->fromHost.doTakeReceivedPacketImpl(buffer);
  }
  return -EINVAL;
}
