/**
 * \file adxl345.h
 *
 * Implementation for manipulating the ADXL345 accelerometer sensor.
 */

#include "adxl345.h"
#include "adxl345_flags.h"
#include "adxl345_transport_types.h"
#include <adxl345_spi_types.h>
#include <errno.h>

static void readRegister(struct Adxl345_Handle *handle,
                         enum Adxl345Flags_Address addr,
                         union Adxl345Register *reg) {
  union Adxl345Transport_TxFrame txFrame = {.asAddress =
                                                addr | Adxl345Spi_RwFlags_read};
  union Adxl345Transport_RxFrame rxFrame = {0};
  handle->transmitReceiveFrame(&txFrame, &rxFrame, 1);
  *reg = rxFrame.asRegister;
}

static void writeRegister(struct Adxl345_Handle *handle,
                          enum Adxl345Flags_Address addr,
                          const union Adxl345Register *reg) {
  union Adxl345Transport_TxFrame txFrame = {.asAddress = addr};
  txFrame.asPaddedRegister.asRegister = *reg;
  handle->transmitFrame(&txFrame, 2, Adxl345Spi_Cs_modify,
                        Adxl345Spi_RwFlags_write);
}

int Adxl345_init(struct Adxl345_Handle *handle) {

  { // data format
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asDataFormat = {
            .range = Adxl345Flags_DataFormat_Range_16g,
            .justify = Adxl345Flags_DataFormat_Justify_lsbRight,
            .fullRes = Adxl345Flags_DataFormat_FullResBit_fullRes_4mg,
            ._zeroD4 = 0,
            .intInvert = Adxl345Flags_DataFormat_IntInvert_activeHigh,
            .spi = Adxl345Flags_DataFormat_SpiBit_4wire,
            .selfTest = Adxl345Flags_DataFormat_SelfTest_disableForce}};
    tx_frame.asAddress = Adxl345Flags_Address_dataFormat;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  { // bandwidth rate
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asBwRate = {
            .rate = Adxl345Flags_BwRate_Rate_normalPowerOdr3200,
            .lowPower = Adxl345Flags_BwRate_LowPower_normal,
            ._zeroD5 = 0,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Flags_Address_bwRate;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  { // fifo control
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asFifoCtl = {
            .samples = ADXL345_WATERMARK_LEVEL,
            .trigger = Adxl345Flags_FifoCtl_Trigger_int1,
            .fifoMode = Adxl345Flags_FifoCtl_FifoMode_fifo}};
    tx_frame.asAddress = Adxl345Flags_Address_fifoCtl;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  { // power control
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asPowerControl = {
            .wakeup = Adxl345Flags_PowerCtl_Wakeup_8Hz,
            .sleep = Adxl345Flags_PowerCtl_Sleep_normalMode,
            .measure = Adxl345Flags_PowerCtl_Measure_standby,
            .autoSleep = Adxl345Flags_PowerCtl_AutoSleep_disabled,
            .link = Adxl345Flags_PowerCtl_Link_concurrent,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Flags_Address_powerCtl;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  { // interrupt map: watermark -> INT1, overrun -> INT2
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asIntMap = {
            .overrun = Adxl345Flags_IntMap_Overrun_int2,
            .watermark = Adxl345Flags_IntMap_Watermark_int1,
            .freeFall = Adxl345Flags_IntMap_FreeFall_int1,
            .inactivity = Adxl345Flags_IntMap_Inactivity_int1,
            .activity = Adxl345Flags_IntMap_Activity_int1,
            .doubleTap = Adxl345Flags_IntMap_DoubleTap_int1,
            .singleTap = Adxl345Flags_IntMap_SingleTap_int1,
            .dataReady = Adxl345Flags_IntMap_DataReady_int1}};
    tx_frame.asAddress = Adxl345Flags_Address_intMap;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  { // interrupt enable: watermark + overrun
    union Adxl345Transport_TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asIntEnable = {
            .overrun = Adxl345Flags_IntEnable_Overrun_enable,
            .watermark = Adxl345Flags_IntEnable_Watermark_enable,
            .freeFall = Adxl345Flags_IntEnable_FreeFall_disable,
            .inactivity = Adxl345Flags_IntEnable_Inactivity_disable,
            .activity = Adxl345Flags_IntEnable_Activity_disable,
            .doubleTap = Adxl345Flags_IntEnable_DoubleTap_disable,
            .singleTap = Adxl345Flags_IntEnable_SingleTap_disable,
            .dataReady = Adxl345Flags_IntEnable_DataReady_disable}};
    tx_frame.asAddress = Adxl345Flags_Address_intEnable;
    handle->transmitFrame(&tx_frame, 2, Adxl345Spi_Cs_modify,
                          Adxl345Spi_RwFlags_write);
  }

  return 0;
}

int Adxl345_setOutputDataRate(struct Adxl345_Handle *handle, uint8_t rate) {
  switch ((enum Adxl345Flags_BwRate_Rate)rate) {
  case Adxl345Flags_BwRate_Rate_normalPowerOdr3200:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr1600:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr800:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr400:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr200:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr100:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr50:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr25:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr12_5:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr6_25:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr3_13:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr1_56:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr0_78:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr0_39:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr0_20:
  case Adxl345Flags_BwRate_Rate_normalPowerOdr0_10: {
    union Adxl345Register reg = {0};
    readRegister(handle, Adxl345Flags_Address_bwRate, &reg);
    reg.asBwRate.rate = (enum Adxl345Flags_BwRate_Rate)rate;
    writeRegister(handle, Adxl345Flags_Address_bwRate, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

int Adxl345_getOutputDataRate(struct Adxl345_Handle *handle,
                              enum Adxl345Flags_BwRate_Rate *rate) {

  if (NULL == rate)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(handle, Adxl345Flags_Address_bwRate, &reg);
  *rate = (enum Adxl345Flags_BwRate_Rate)reg.asBwRate.rate;

  return 0;
}

int Adxl345_setRange(struct Adxl345_Handle *handle, uint8_t range) {
  switch ((enum Adxl345Flags_DataFormat_Range)range) {
  case Adxl345Flags_DataFormat_Range_2g:
  case Adxl345Flags_DataFormat_Range_4g:
  case Adxl345Flags_DataFormat_Range_8g:
  case Adxl345Flags_DataFormat_Range_16g: {
    union Adxl345Register reg = {0};
    readRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
    reg.asDataFormat.range = (enum Adxl345Flags_BwRate_Rate)range;
    writeRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

int Adxl345_getRange(struct Adxl345_Handle *handle,
                     enum Adxl345Flags_DataFormat_Range *range) {
  if (NULL == range)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
  *range = (enum Adxl345Flags_DataFormat_Range)reg.asDataFormat.range;

  return 0;
}

int Adxl345_getScale(struct Adxl345_Handle *handle,
                     enum Adxl345Flags_DataFormat_FullResBit *scale) {
  if (NULL == scale)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
  *scale = (enum Adxl345Flags_DataFormat_FullResBit)reg.asDataFormat.fullRes;

  return 0;
}

int Adxl345_setScale(struct Adxl345_Handle *handle, uint8_t scale) {
  switch ((enum Adxl345Flags_DataFormat_FullResBit)scale) {
  case Adxl345Flags_DataFormat_FullResBit_10bit:
  case Adxl345Flags_DataFormat_FullResBit_fullRes_4mg: {
    union Adxl345Register reg = {0};
    readRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
    reg.asDataFormat.fullRes = (enum Adxl345Flags_DataFormat_FullResBit)scale;
    writeRegister(handle, Adxl345Flags_Address_dataFormat, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

void Adxl345_setPowerCtlStandby(struct Adxl345_Handle *handle) {
  union Adxl345Register reg = {0};
  readRegister(handle, Adxl345Flags_Address_powerCtl, &reg);
  reg.asPowerControl.measure = Adxl345Flags_PowerCtl_Measure_standby;
  writeRegister(handle, Adxl345Flags_Address_powerCtl, &reg);
}

void Adxl345_setPowerCtlMeasure(struct Adxl345_Handle *handle) {
  union Adxl345Register reg = {0};
  readRegister(handle, Adxl345Flags_Address_powerCtl, &reg);
  reg.asPowerControl.measure = Adxl345Flags_PowerCtl_Measure_measure;
  writeRegister(handle, Adxl345Flags_Address_powerCtl, &reg);
}

int Adxl345_getAcceleration(struct Adxl345_Handle *handle,
                            struct Adxl345Transport_Acceleration *acc) {
  if (NULL == acc)
    return -EINVAL;

  union Adxl345Transport_TxFrame tx_frame = {.asAddress =
                                                 Adxl345Flags_Address_dataX0};
  union Adxl345Transport_RxFrame rx_frame = {0};
  handle->transmitReceiveFrame(&tx_frame, &rx_frame, 6);
  *acc = rx_frame.asAcceleration;

  return 0;
}
