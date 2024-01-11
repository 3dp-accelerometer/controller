/**
 * \file host_transport_impl.c
 *
 * Controller to IN USB endpoint (host) transport implementation.
 */

#include "fw/host_transport_impl.h"
#include "usbd_cdc_if.h"
#include <controller.h>
#include <errno.h>
#include <host_transport.h>
#include <host_transport_types.h>

extern struct Controller_Handle controllerHandle;

enum HostTransport_Status HostTransportImpl_transmit(uint8_t *buffer,
                                                     uint16_t len) {
  switch (CDC_Transmit_FS(buffer, len)) {
  case USBD_OK:
    return HostTransport_Status_Ok;
  case USBD_BUSY:
    return HostTransport_Status_Busy;
  case USBD_EMEM:
  case USBD_FAIL:
    return HostTransport_Status_Fail;
  default:
    return HostTransport_Status_Undefined;
  }
}

int HostTransportImpl_onPacketReceived(uint8_t *buffer) {
  if (NULL == buffer)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
  case Transport_HeaderId_Rx_GetFirmwareVersion:
    return controllerHandle.hostOnRequestGetFirmwareVersion();
  case Transport_HeaderId_Rx_GetOutputDataRate:
    return controllerHandle.hostOnRequestGetOutputDataRate();
  case Transport_HeaderId_Rx_SetOutputDataRate:
    return controllerHandle.hostOnRequestSetOutputDatatRate(
        request->asRxFrame.asSetOutputDataRate.rate);
  case Transport_HeaderId_Rx_GetRange:
    return controllerHandle.hostOnRequestGetRange();
  case Transport_HeaderId_Rx_SetRange:
    return controllerHandle.hostOnRequestSetRange(
        request->asRxFrame.asSetRange.range);
  case Transport_HeaderId_Rx_GetScale:
    return controllerHandle.hostOnRequestGetScale();
  case Transport_HeaderId_Rx_SetScale:
    return controllerHandle.hostOnRequestSetScale(
        request->asRxFrame.asSetScale.scale);
  case Transport_HeaderId_Rx_GetDeviceSetup:
    return controllerHandle.hostOnRequestGetDeviceSetup();
  case Transport_HeaderId_Rx_DeviceReboot: {
    controllerHandle.controllerRequestReboot();
    return 0;
  }
  case Transport_HeaderId_Rx_SamplingStart:
    return controllerHandle.hostOnRequestSamplingStart(
        request->asRxFrame.asSamplingStart.max_samples_count);
  case Transport_HeaderId_Rx_SamplingStop:
    return controllerHandle.hostOnRequestSamplingStop();

  default:
    return -EINVAL;
  }
}
