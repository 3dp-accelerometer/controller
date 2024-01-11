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

enum HostTransport_Status HostTransportImpl_doTransmitImpl(uint8_t *buffer,
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

int HostTransportImpl_onTakeReceivedImpl(uint8_t *buffer) {
  if (NULL == buffer)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
  case Transport_HeaderId_Rx_GetFirmwareVersion:
    return controllerHandle.host.onRequestGetFirmwareVersion();
  case Transport_HeaderId_Rx_GetOutputDataRate:
    return controllerHandle.host.onRequestGetOutputDataRate();
  case Transport_HeaderId_Rx_SetOutputDataRate:
    return controllerHandle.host.onRequestSetOutputDatatRate(
        request->asRxFrame.asSetOutputDataRate.rate);
  case Transport_HeaderId_Rx_GetRange:
    return controllerHandle.host.onRequestGetRange();
  case Transport_HeaderId_Rx_SetRange:
    return controllerHandle.host.onRequestSetRange(
        request->asRxFrame.asSetRange.range);
  case Transport_HeaderId_Rx_GetScale:
    return controllerHandle.host.onRequestGetScale();
  case Transport_HeaderId_Rx_SetScale:
    return controllerHandle.host.onRequestSetScale(
        request->asRxFrame.asSetScale.scale);
  case Transport_HeaderId_Rx_GetDeviceSetup:
    return controllerHandle.host.onRequestGetDeviceSetup();
  case Transport_HeaderId_Rx_DeviceReboot: {
    controllerHandle.requestReboot();
    return 0;
  }
  case Transport_HeaderId_Rx_SamplingStart:
    return controllerHandle.host.onRequestSamplingStart(
        request->asRxFrame.asSamplingStart.max_samples_count);
  case Transport_HeaderId_Rx_SamplingStop:
    return controllerHandle.host.onRequestSamplingStop();

  default:
    return -EINVAL;
  }
}
