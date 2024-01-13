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
 * @param numBytes expected size of received data
 * @param applyCs whether or not to set nCS before and clear nCS after
 * transmission
 *
 * @return -EINVAL if invalid args, -EIO on RX error, 0 otherwise
 */
static int receiveFrame(union Adxl345Transport_RxFrame *frame,
                        // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
                        uint8_t numBytes, enum Adxl345Spi_Cs applyCs) {

  if (NULL == frame) {
    return -EINVAL;
  }
  const uint32_t timeoutMs = 10;

  if (Adxl345Spi_Cs_modify == applyCs) {
    ncsSet();
    if (0 != HAL_SPI_Receive(&hspi1, (uint8_t *)frame, numBytes, timeoutMs)) {
      return -EIO;
    }
    ncsClear();
  } else {
    if (0 != HAL_SPI_Receive(&hspi1, (uint8_t *)frame, numBytes, timeoutMs)) {
      return -EIO;
    }
  }

  return 0;
}

int Adxl345TransportImpl_doTransmitFrameImpl(
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    const union Adxl345Transport_TxFrame *frame, uint8_t numBytes,
    enum Adxl345Spi_Cs applyCs, enum Adxl345Spi_RwFlags rwFlag) {

  if (NULL == frame) {
    return -EINVAL;
  }

  union Adxl345Transport_TxFrame tx_frame = {.asWord = frame->asWord};

  const uint32_t timeoutMs = 10;
  tx_frame.asAddress |= rwFlag;

  if (Adxl345Spi_Cs_modify == applyCs) {
    ncsSet();
    if (0 !=
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&tx_frame, numBytes, timeoutMs)) {
      return -EIO;
    }
    ncsClear();
  } else {
    if (0 !=
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&tx_frame, numBytes, timeoutMs)) {
      return -EIO;
    }
  }
  return 0;
}

int Adxl345TransportImpl_doTransmitReceiveFrameImpl(
    const union Adxl345Transport_TxFrame *txFrame,
    union Adxl345Transport_RxFrame *rxFrame, uint8_t numBytesReceive) {

  if (NULL == txFrame || NULL == rxFrame) {
    return -EINVAL;
  }

  const uint8_t multiByte = (numBytesReceive > 1)
                                ? Adxl345Spi_RwFlags_multiByte
                                : Adxl345Spi_RwFlags_singleByte;
  ncsSet();
  Adxl345TransportImpl_doTransmitFrameImpl(txFrame, 1, Adxl345Spi_Cs_untouched,
                                           (uint8_t)Adxl345Spi_RwFlags_read |
                                               multiByte);
  receiveFrame(rxFrame, numBytesReceive, Adxl345Spi_Cs_untouched);
  ncsClear();

  return 0;
}
