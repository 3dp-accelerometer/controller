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
enum Adxl345Spi_Cs {
  Adxl345Spi_Cs_modify =
      0, ///< modifies chip select line before/after transaction
  Adxl345Spi_Cs_untouched ///< leaves chip select untouched before/after
                          ///< transaction
};

/**
 * Read/write flags for SPI communication
 */
enum Adxl345Spi_RwFlags {
  Adxl345Spi_RwFlags_read = 0x80U,       ///< indicates SPI read
  Adxl345Spi_RwFlags_write = 0x00U,      ///< indicates SPI write
  Adxl345Spi_RwFlags_multiByte = 0x40U,  ///< indicates multibyte transfer
  Adxl345Spi_RwFlags_singleByte = 0x00U, ///< indicates single byte transfer
};
