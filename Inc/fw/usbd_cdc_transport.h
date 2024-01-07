/**
 * \file usbd_cdc_transport.h
 *
 * Controller to IN USB endpoint (host) transport API.
 */

#pragma once
#include <inttypes.h>

struct Adxl345TP_Acceleration;

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
 * @param buffer received package (as a whole, must not be fragmented)
 * @param length received package length
 * @return 0 on success, EINVAL otherwise
 */
int TransportRx_Process(uint8_t *buffer, const uint32_t *length);

/**
 * Transmits device configuration TransportTx_DeviceSetup to the IN endpoint of
 * host.
 */
void TransportTx_SamplingSetup();

/**
 * Transmits firmware version TransportTx_FirmwareVersion to the IN endpoint of
 * host.
 */
void TransportTx_FirmwareVersion();

/**
 * Transmits sampling started package TransportTx_SamplingStarted to the IN
 * endpoint of host.
 */
void TransportTx_SamplingStarted(uint16_t max_samples);

/**
 * Transmits sampling finished package TransportTx_SamplingFinished to the IN
 * endpoint of host.
 */
void TransportTx_SamplingFinished();

/**
 * Transmits sampling stopped package TransportTx_SamplingStopped to the IN
 * endpoint of host.
 */
void TransportTx_SamplingStopped();

/**
 * Transmits sampling aborted package TransportTx_SamplingAborted to the IN
 * endpoint of host.
 */
void TransportTx_SamplingAborted();

/**
 * Transmits FiFo overflow package TransportTx_FifoOverflow to the IN endpoint
 * of host.
 */
void TransportTx_FifoOverflow();

/**
 * Forwards acceleration data block to the IN endpoint of host.
 *
 * Triggers sending data to the the IN endpoint.
 * If USB is busy the data is buffered for a later transmission.
 * To consume all buffered data this function shall be called until ENODATA is
 * returned (data and count must be NULL and 0).
 *
 * @param data tx buffer or NULL to consume buffered data
 * @param count buffer size or 0 to consume buffered data
 * @param start_index where to start from within data
 * @return
 *   - 0 on success (data send in first run),
 *   - EBUSY if data was buffered,
 *   - ENODATA if no buffered data available (all data sent),
 *   - -EINVAL otherwise
 */
int TransportTx_AccelerationBuffer(struct Adxl345TP_Acceleration *data,
                                   uint8_t count, uint16_t start_index);
