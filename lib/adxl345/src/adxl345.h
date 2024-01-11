/**
 * \file adxl345.h
 *
 * API for manipulating the ADXL345 accelerometer sensor.
 */

#pragma once

#include <assert.h>
#include <inttypes.h>

struct Adxl345Transport_Acceleration;
union Adxl345Transport_TxFrame;
union Adxl345Transport_RxFrame;
enum Adxl345Spi_Cs;
enum Adxl345Spi_RwFlags;
enum Adxl345Flags_BwRate_Rate;
enum Adxl345Flags_DataFormat_Range;
enum Adxl345Flags_DataFormat_FullResBit;

//@{
/**
 * ADXL345 watermark configuration.
 *
 * Sensor provides 32 FiFo entries 2-bytes each.
 * Recommended watermark level is about 75% of FiFo.
 * Setting the watermark level too high might leave too little room
 * for buffering until respective routine clears the FiFo.
 * This might increases the probability of FiFo overflow (which is asserted
 * anyway).
 */
#define ADXL345_FIFO_ENTRIES 32 // 32 * (X, Y, Z); 2 bytes each coordinate
#define ADXL345_WATERMARK_LEVEL 24
//@}

static_assert(ADXL345_WATERMARK_LEVEL <= 32,
              "ERROR: maximum allowed watermark level: 32");
static_assert(ADXL345_WATERMARK_LEVEL >= 0,
              "ERROR: minimum allowed watermark level: 0");

/**
 * The HW handle pointing to the underlying SPI communication implementation.
 */
struct Adxl345_Handle {
  /**
   * Sending payload to SPI bus.
   *
   * Context: main() and interrupts
   *
   * @return
   */
  int (*const transmitFrame)(union Adxl345Transport_TxFrame *, uint8_t,
                             enum Adxl345Spi_Cs, enum Adxl345Spi_RwFlags);

  /**
   * reading from sensor: send request to SPI then receive from SPI
   *
   * Context: main() and interrupts
   */
  int (*const transmitReceiveFrame)(union Adxl345Transport_TxFrame *,
                                    union Adxl345Transport_RxFrame *, uint8_t);
};

/**
 * Sensor initialization.
 *
 * Shall be called after sensor power cycle.
 * Might be called at runtime as well.
 */
int Adxl345_init(struct Adxl345_Handle *handle);

//@{
/**
 * Output data rate (ODR) setter/getter.
 *
 * \see Adxl345Flags_BwRate_Rate
 */
int Adxl345_getOutputDataRate(struct Adxl345_Handle *handle,
                              enum Adxl345Flags_BwRate_Rate *rate);
int Adxl345_setOutputDataRate(struct Adxl345_Handle *handle, uint8_t rate);
//@}

//@{
/**
 * Range setter/getter.
 *
 * The range specifies which bandwidth of acceleration the sensor is going to
 * measure (i.e. +/-4g).
 * \see Adxl345Flags_DataFormat_Range
 */
int Adxl345_getRange(struct Adxl345_Handle *handle,
                     enum Adxl345Flags_DataFormat_Range *range);
int Adxl345_setRange(struct Adxl345_Handle *handle, uint8_t range);
//@}

//@{
/**
 * Scale setter/getter.
 *
 * The scale denotes the amount of acceleration per bit.
 * For example: 1LSB=4mg if Adxl345Register_DataFormat_FullResBit_fullRes_4mg
 * is configured.
 * \see Adxl345Flags_DataFormat_FullResBit
 */
int Adxl345_getScale(struct Adxl345_Handle *handle,
                     enum Adxl345Flags_DataFormat_FullResBit *scale);
int Adxl345_setScale(struct Adxl345_Handle *handle, uint8_t scale);
//@}

//@{
/**
 * Sensor power mode.
 *
 * If sensor should be operational then power mode "measure" shall be
 * configured.
 */
void Adxl345_setPowerCtlStandby(struct Adxl345_Handle *handle);
void Adxl345_setPowerCtlMeasure(struct Adxl345_Handle *handle);
//@}

/**
 * Read next acceleration from FiFo.
 *
 * \see Adxl345Transport_Acceleration
 */
int Adxl345_getAcceleration(struct Adxl345_Handle *handle,
                            struct Adxl345Transport_Acceleration *acc);
