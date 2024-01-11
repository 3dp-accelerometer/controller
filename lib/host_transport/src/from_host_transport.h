/**
 * \file from_host_transport.h
 *
 * API for processing data from host to controller.
 */

#pragma once
#include <inttypes.h>

struct HostTransport_Handle;

/**
 * Processes received package from the OUT endpoint of host.
 *
 * Shall be called in CDC_Receive_FS(uint8_t* Buf, uint32_t *Len).
 * Performs simple check on the recieved buffer and forwards of the data package
 * header ID is one of:
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
 * @param handle host transport pimpl
 * @param buffer received package (as a whole, must not be fragmented)
 * @param length received package length
 * @return
 *   - -EINVAL on invalid arguments
 *   - HostTransport_Handle.onReceived(uint8_t *, uint32_t *) otherwise
 */
int TransportRx_Process(struct HostTransport_Handle *handle, uint8_t *buffer,
                        uint16_t length);
