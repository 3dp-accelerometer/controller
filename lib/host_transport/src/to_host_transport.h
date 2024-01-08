/**
 * \file to_host_transport.h
 *
 * API for transporting data from controller to host.
 */

#pragma once
#include <inttypes.h>

struct Adxl345_Handle;
struct Adxl345Transport_Acceleration;

/**
 * Transmits device configuration TransportTx_DeviceSetup to the IN endpoint of
 * host.
 *
 * @param host_handle host transport pimpl
 * @param sensor_handle acceleration sensor transport pimpl
 */
void TransportTx_SamplingSetup(struct HostTransport_Handle *host_handle,
                               struct Adxl345_Handle *sensor_handle);

/**
 * Transmits firmware version TransportTx_FirmwareVersion to the IN endpoint of
 * host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_FirmwareVersion(struct HostTransport_Handle *handle);

/**
 * Transmits sampling started package TransportTx_SamplingStarted to the IN
 * endpoint of host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_SamplingStarted(struct HostTransport_Handle *handle,
                                 uint16_t max_samples);

/**
 * Transmits sampling finished package TransportTx_SamplingFinished to the IN
 * endpoint of host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_SamplingFinished(struct HostTransport_Handle *handle);

/**
 * Transmits sampling stopped package TransportTx_SamplingStopped to the IN
 * endpoint of host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_SamplingStopped(struct HostTransport_Handle *handle,
                                 struct Adxl345_Handle *sensor_handle);

/**
 * Transmits sampling aborted package TransportTx_SamplingAborted to the IN
 * endpoint of host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_SamplingAborted(struct HostTransport_Handle *handle);

/**
 * Transmits FiFo overflow package TransportTx_FifoOverflow to the IN endpoint
 * of host.
 *
 * @param handle host transport pimpl
 */
void TransportTx_FifoOverflow(struct HostTransport_Handle *handle);

/**
 * Forwards acceleration data block to the IN endpoint of host.
 *
 * Triggers sending data to the the IN endpoint.
 * If USB is busy the data is buffered for a later transmission.
 * To consume all buffered data this function shall be called until ENODATA is
 * returned (data and count must be NULL and 0).
 *
 * @param handle host transport pimpl
 * @param data tx buffer or NULL to consume buffered data
 * @param count buffer size or 0 to consume buffered data
 * @param start_index where to start from within data
 * @return
 *   - 0 on success (data send in first run),
 *   - EBUSY if data was buffered,
 *   - ENODATA if no buffered data available (all data sent),
 *   - -EINVAL otherwise
 */
int TransportTx_AccelerationBuffer(struct HostTransport_Handle *handle,
                                   struct Adxl345Transport_Acceleration *data,
                                   uint8_t count, uint16_t start_index);
