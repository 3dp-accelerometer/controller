/**
 * \file from_host_transport.h
 *
 * API for processing data from host to controller.
 */

#pragma once
#include <inttypes.h>

struct Adxl345_Handle;
struct HostTransport_Handle;

/**
 * Processes received package from the OUT endpoint of host.
 *
 * Shall be called in CDC_Receive_FS(uint8_t* Buf, uint32_t *Len).
 * Handles requests, namely received packets with RX header ID one of:
 *
 *   - TransportHeader_Id_Rx_GetFirmwareVersion
 *   - TransportHeader_Id_Rx_GetOutputDataRate
 *   - TransportHeader_Id_Rx_SetOutputDataRa
 *   - TransportHeader_Id_Rx_GetRange
 *   - TransportHeader_Id_Rx_SetRange
 *   - TransportHeader_Id_Rx_GetScale
 *   - TransportHeader_Id_Rx_SetScale
 *   - TransportHeader_Id_Rx_GetDeviceSetup
 *   - TransportHeader_Id_Rx_DeviceReboot
 *   - TransportHeader_Id_Rx_SamplingStart
 *   - TransportHeader_Id_Rx_SamplingStop
 *
 * @param host_handle host transport pimpl
 * @param sensor_handle acceleration sensor transport pimpl
 * @param buffer received package (as a whole, must not be fragmented)
 * @param length received package length
 * @return 0 on success, EINVAL otherwise
 */
int TransportRx_Process(struct HostTransport_Handle *host_handle,
                        struct Adxl345_Handle *sensor_handle,
                        uint8_t *buffer,
                        const uint32_t *length);
