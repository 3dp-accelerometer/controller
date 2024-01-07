/**
 * \file adxl345.h
 *
 * Accelerometer API for manipulating an ADXL345 sensor.
 */
#pragma once

#include <assert.h>
#include <inttypes.h>

enum Adxl345Register_BwRate_Rate;
enum Adxl345Register_DataFormat_Range;
enum Adxl345Register_DataFormat_FullResBit;
struct Adxl345TP_Acceleration;

/**
 * Sensor initialization.
 *
 * Shall be called after sensor power cycle.
 * Might be called at runtime as well.
 */
int Adxl345_init();

//@{
/**
 * Output data rate (ODR) setter/getter.
 *
 * \see Adxl345Register_BwRate_Rate
 */
int Adxl345_getOutputDataRate(enum Adxl345Register_BwRate_Rate *rate);
int Adxl345_setOutputDataRate(uint8_t rate);
//@}

//@{
/**
 * Range setter/getter.
 *
 * The range specifies which bandwidth of acceleration the sensor is going to
 * measure (i.e. +/-4g).
 * \see Adxl345Register_DataFormat_Range
 */
int Adxl345_getRange(enum Adxl345Register_DataFormat_Range *range);
int Adxl345_setRange(uint8_t range);
//@}

//@{
/**
 * Scale setter/getter.
 *
 * The scale denotes the amount of acceleration per bit.
 * For example: 1LSB=4mg if Adxl345Register_DataFormat_FullResBit_fullRes_4mg
 * is configured.
 * \see Adxl345Register_DataFormat_FullResBit
 */
int Adxl345_getScale(enum Adxl345Register_DataFormat_FullResBit *scale);
int Adxl345_setScale(uint8_t scale);
//@}

//@{
/**
 * Sensor power mode.
 *
 * If sensor should be operational then power mode "measure" shall be
 * configured.
 */
void Adxl345_setPowerCtlStandby();
void Adxl345_setPowerCtlMeasure();
//@}

/**
 * Read next acceleration from FiFo.
 *
 * \see Adxl345_Acceleration
 */
int Adxl345_getAcceleration(struct Adxl345TP_Acceleration *acc);
