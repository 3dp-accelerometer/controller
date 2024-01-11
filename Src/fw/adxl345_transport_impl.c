/**
 * \file adxl345_transport_impl.c
 *
 * Implements hardware specific ADXL345 API.
 */

#include "fw/adxl345_transport_impl.h"
#include "gpio.h"
#include "spi.h"
#include <adxl345_spi_types.h>
#include <adxl345_transport_types.h>
#include <errno.h>

/**
 * Sets the chip select (CS) line accordingly: nCS is active low.
 */
static void ncsSet() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_RESET);
}

/**
 * Clears the chip select (CS) line accordingly: nCS is inactive high.
 */
static void ncsClear() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_SET);
}

/**
 * Receives frame from ADXL345.
 *
 * \see Adxl345TransportImpl_transmitReceiveFrame(union Adxl345TP_TxFrame *,
 * union Adxl345TP_RxFrame *, uint8_t)
 *
 * @param frame receive buffer
 * @param num_bytes expected size of received data
 * @param applyCs whether or not to set nCS before and clear nCS after
 * transmission
 *
 * @return -EINVAL if invalid args, -EIO on RX error, 0 otherwise
 */
static int receiveFrame(union Adxl345Transport_RxFrame *frame,
                        uint8_t num_bytes, enum Adxl345Spi_Cs applyCs) {

  if (NULL == frame)
    return -EINVAL;

  if (Adxl345Spi_Cs_modify == applyCs) {
    ncsSet();
    if (0 != HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10))
      return -EIO;
    ncsClear();
  } else {
    if (0 != HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10))
      return -EIO;
  }

  return 0;
}

int Adxl345TransportImpl_doTransmitFrameImpl(
    union Adxl345Transport_TxFrame *frame, uint8_t numBytes,
    enum Adxl345Spi_Cs applyCs, enum Adxl345Spi_RwFlags rwFlag) {

  if (NULL == frame)
    return -EINVAL;

  frame->asAddress |= rwFlag;

  if (Adxl345Spi_Cs_modify == applyCs) {
    ncsSet();
    if (0 != HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, numBytes, 10))
      return -EIO;
    ncsClear();
  } else {
    if (0 != HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, numBytes, 10))
      return -EIO;
  }
  return 0;
}

int Adxl345TransportImpl_doTransmitReceiveFrameImpl(
    union Adxl345Transport_TxFrame *tx_frame,
    union Adxl345Transport_RxFrame *rx_frame, uint8_t num_bytes_receive) {

  if (NULL == tx_frame || NULL == rx_frame)
    return -EINVAL;

  const uint8_t multiByte = (num_bytes_receive > 1)
                                ? Adxl345Spi_RwFlags_multiByte
                                : Adxl345Spi_RwFlags_singleByte;
  ncsSet();
  Adxl345TransportImpl_doTransmitFrameImpl(tx_frame, 1, Adxl345Spi_Cs_untouched,
                                           Adxl345Spi_RwFlags_read | multiByte);
  receiveFrame(rx_frame, num_bytes_receive, Adxl345Spi_Cs_untouched);
  ncsClear();

  return 0;
}
