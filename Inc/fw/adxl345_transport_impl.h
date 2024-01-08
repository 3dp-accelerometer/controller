/**
 * \file adxl345_transport_impl.h
 *
 * API of the underlying SPI communication to the ADXL345 sensor.
 */

#pragma once

#include <adxl345_spi_types.h>
#include <adxl345_transport_types.h>
#include <inttypes.h>

struct Adxl345_Handle;

/**
 * Sends one single frame to ADXL345 via SPI interface.
 *
 * @param frame the payload to send via SPI
 * @param num_bytes size of frame
 * @param apply_cs whether or not to set nCS before and clear nCS after
 * transmission
 * @param rw_flag flag to indicate whether this transaction is write or read
 *   \see Adxl345TransportImpl_transmitReceiveFrame(union Adxl345TP_TxFrame *,
 * union Adxl345TP_RxFrame *, uint8_t)
 *
 * @return -EINVAL if invalid args, -EIO on TX error, 0 otherwise
 */
int Adxl345TransportImpl_transmitFrame(union Adxl345Transport_TxFrame *frame,
                                       uint8_t num_bytes,
                                       enum Adxl345Spi_Cs apply_cs,
                                       enum Adxl345Spi_RwFlags rw_flag);

/**
 * Transmits and receives frames (read transaction).
 *
 * Note: This automatically sets nCS before TX and clears nCS after completed
 * RX.
 *
 * @param tx_frame data to send
 * @param rx_frame receive data
 * @param num_bytes_receive expected size of received data
 *
 * @return -EINVAL if invalid args, -EIO on TX/RX error, 0 otherwise
 */
int Adxl345TransportImpl_transmitReceiveFrame(union Adxl345Transport_TxFrame *tx_frame,
                                              union Adxl345Transport_RxFrame *rx_frame,
                                              uint8_t num_bytes_receive);

/**
 * Initializes the handle's read/write pimpl.
 *
 * @param handle
 * @return -EINVAL or error, 0 otherwise
 */
int Adxl345TransportImpl_initHandle(struct Adxl345_Handle *handle);
