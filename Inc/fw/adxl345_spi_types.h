/**
 * \file adxl345_spi_types.h
 *
 * ADXL345 SPI TX/RX constants.
 */

#pragma once

#include <assert.h>
#include <inttypes.h>

/* TX/RX SPI Constants
 * -----------------------------------------------------------*/

/**
 * Chip Select flags for SPI communication.
 */
enum Adxl345SPI_CS { Adxl345SPI_CS_modify = 0, Adxl345SPI_CS_untouched };

/**
 * Read/write flags for SPI communication
 */
enum Adxl345SPI_RWFlags {
  Adxl345SPI_RWFlags_read = 0x80,
  Adxl345SPI_RWFlags_write = 0x00,
  Adxl345SPI_RWFlags_multiByte = 0x40,
  Adxl345SPI_RWFlags_singleByte = 0x00,
};
