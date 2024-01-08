/**
 * \file host_transport_impl.c
 *
 * Controller to IN USB endpoint (host) transport implementation.
 */

#include "usbd_cdc_if.h"
#include <errno.h>
#include <host_transport.h>

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
    return HostTransport_Status_Fail;
  default:
    return HostTransport_Status_Undefined;
  }
}

int HostTransportImpl_initHandle(struct HostTransport_Handle *handle) {
  if (NULL == handle)
    return -EINVAL;
  handle->transmit = CDC_Transmit_FS;
  return 0;
}
