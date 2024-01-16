/**
 * \file host_transport.c
 */

#include "host_transport.h"

void Transport_resetBuffer(struct HostTransport_Handle *handle) {
  handle->toHost.largestTxChunkBytes = 0;
  Ringbuffer_reset(&handle->toHost.ringbuffer);
}
