#include "adxl345.h"
#include "gpio.h"
#include "spi.h"
#include <errno.h>

static void ncsSet() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_RESET);
}

static void ncsClear() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_SET);
}

static void transmitFrame(union Adxl345TxFrame *frame, uint8_t num_bytes,
                          enum Adxl345CS apply_cs,
                          enum Adxl345RWFlags rw_flag) {
  frame->asAddress |= rw_flag;

  if (Adxl345CS_modify == apply_cs) {
    ncsSet();
    HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, num_bytes, 10);
    ncsClear();
  } else {
    HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, num_bytes, 10);
  }
}

static void receiveFrame(union Adxl345RxFrame *frame, uint8_t num_bytes,
                         enum Adxl345CS apply_cs) {

  if (Adxl345CS_modify == apply_cs) {
    ncsSet();
    HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10);
    ncsClear();
  } else {
    HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10);
  }
}

static void transmitReceiveFrame(union Adxl345TxFrame *tx_frame,
                                 union Adxl345RxFrame *rx_frame,
                                 uint8_t num_bytes_receive) {
  const uint8_t multiByte =
      num_bytes_receive > 1 ? Adxl345RWFlags_multiByte : 0;
  ncsSet();
  transmitFrame(tx_frame, 1, Adxl345CS_untouched,
                Adxl345RWFlags_read | multiByte);
  receiveFrame(rx_frame, num_bytes_receive, Adxl345CS_untouched);
  ncsClear();
}

static void readRegister(enum Adxl345Register_Address addr,
                         union Adxl345Register *reg) {
  union Adxl345TxFrame txFrame = {.asAddress = addr | Adxl345RWFlags_read};
  union Adxl345RxFrame rxFrame = {0};
  transmitReceiveFrame(&txFrame, &rxFrame, 1);
  *reg = rxFrame.asRegister;
}

static void writeRegister(enum Adxl345Register_Address addr,
                          union Adxl345Register *reg) {
  union Adxl345TxFrame txFrame = {.asAddress = addr};
  txFrame.asPaddedRegister.asRegister = *reg;
  transmitFrame(&txFrame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
}

int Adxl345_init() {

  { // data format
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asDataFormat = {
            .range = Adxl345Register_DataFormat_Range_16g,
            .justify = Adxl345Register_DataFormat_Justify_lsbRight,
            .fullRes = Adxl345Register_DataFormat_FullResBit_fullRes_4mg,
            ._zeroD4 = 0,
            .intInvert = Adxl345Register_DataFormat_IntInvert_activeHigh,
            .spi = Adxl345Register_DataFormat_SpiBit_4wire,
            .selfTest = Adxl345Register_DataFormat_SelfTest_disableForce}};
    tx_frame.asAddress = Adxl345Register_Address_dataFormat;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // bandwidth rate
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asBwRate = {
            .rate = Adxl345Register_BwRate_Rate_normalPowerOdr3200,
            .lowPower = Adxl345Register_BwRate_LowPower_normal,
            ._zeroD5 = 0,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Register_Address_bwRate;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // fifo control
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asFifoCtl = {
            .samples = ADXL345_WATERMARK_LEVEL,
            .trigger = Adxl345Register_FifoCtl_Trigger_int1,
            .fifoMode = Adxl345Register_FifoCtl_FifoMode_fifo}};
    tx_frame.asAddress = Adxl345Register_Address_fifoCtl;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // power control
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asPowerControl = {
            .wakeup = Adxl345Register_PowerCtl_Wakeup_8Hz,
            .sleep = Adxl345Register_PowerCtl_Sleep_normalMode,
            .measure = Adxl345Register_PowerCtl_Measure_standby,
            .autoSleep = Adxl345Register_PowerCtl_AutoSleep_disabled,
            .link = Adxl345Register_PowerCtl_Link_concurrent,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Register_Address_powerCtl;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // interrupt map: watermark -> INT1, overrun -> INT2
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asIntMap = {
            .overrun = Adxl345Register_IntMap_Overrun_int2,
            .watermark = Adxl345Register_IntMap_Watermark_int1,
            .freeFall = Adxl345Register_IntMap_FreeFall_int1,
            .inactivity = Adxl345Register_IntMap_Inactivity_int1,
            .activity = Adxl345Register_IntMap_Activity_int1,
            .doubleTap = Adxl345Register_IntMap_DoubleTap_int1,
            .singleTap = Adxl345Register_IntMap_SingleTap_int1,
            .dataReady = Adxl345Register_IntMap_DataReady_int1}};
    tx_frame.asAddress = Adxl345Register_Address_intMap;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // interrupt enable: watermark + overrun
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asIntEnable = {
            .overrun = Adxl345Register_IntEnable_Overrun_enable,
            .watermark = Adxl345Register_IntEnable_Watermark_enable,
            .freeFall = Adxl345Register_IntEnable_FreeFall_disable,
            .inactivity = Adxl345Register_IntEnable_Inactivity_disable,
            .activity = Adxl345Register_IntEnable_Activity_disable,
            .doubleTap = Adxl345Register_IntEnable_DoubleTap_disable,
            .singleTap = Adxl345Register_IntEnable_SingleTap_disable,
            .dataReady = Adxl345Register_IntEnable_DataReady_disable}};
    tx_frame.asAddress = Adxl345Register_Address_intEnable;
    transmitFrame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  return 0;
}

int Adxl345_setOutputDataRate(uint8_t rate) {
  switch ((enum Adxl345Register_BwRate_Rate)rate) {
  case Adxl345Register_BwRate_Rate_normalPowerOdr3200:
  case Adxl345Register_BwRate_Rate_normalPowerOdr1600:
  case Adxl345Register_BwRate_Rate_normalPowerOdr800:
  case Adxl345Register_BwRate_Rate_normalPowerOdr400:
  case Adxl345Register_BwRate_Rate_normalPowerOdr200:
  case Adxl345Register_BwRate_Rate_normalPowerOdr100:
  case Adxl345Register_BwRate_Rate_normalPowerOdr50:
  case Adxl345Register_BwRate_Rate_normalPowerOdr25:
  case Adxl345Register_BwRate_Rate_normalPowerOdr12_5:
  case Adxl345Register_BwRate_Rate_normalPowerOdr6_25:
  case Adxl345Register_BwRate_Rate_normalPowerOdr3_13:
  case Adxl345Register_BwRate_Rate_normalPowerOdr1_56:
  case Adxl345Register_BwRate_Rate_normalPowerOdr0_78:
  case Adxl345Register_BwRate_Rate_normalPowerOdr0_39:
  case Adxl345Register_BwRate_Rate_normalPowerOdr0_20:
  case Adxl345Register_BwRate_Rate_normalPowerOdr0_10: {
    union Adxl345Register reg = {0};
    readRegister(Adxl345Register_Address_bwRate, &reg);
    reg.asBwRate.rate = (enum Adxl345Register_BwRate_Rate)rate;
    writeRegister(Adxl345Register_Address_bwRate, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

int Adxl345_getOutputDataRate(enum Adxl345Register_BwRate_Rate *rate) {

  if (NULL == rate)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(Adxl345Register_Address_bwRate, &reg);
  *rate = (enum Adxl345Register_BwRate_Rate)reg.asBwRate.rate;

  return 0;
}

int Adxl345_setRange(uint8_t range) {
  switch ((enum Adxl345Register_DataFormat_Range)range) {
  case Adxl345Register_DataFormat_Range_2g:
  case Adxl345Register_DataFormat_Range_4g:
  case Adxl345Register_DataFormat_Range_8g:
  case Adxl345Register_DataFormat_Range_16g: {
    union Adxl345Register reg = {0};
    readRegister(Adxl345Register_Address_dataFormat, &reg);
    reg.asDataFormat.range = (enum Adxl345Register_BwRate_Rate)range;
    writeRegister(Adxl345Register_Address_dataFormat, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

int Adxl345_getRange(enum Adxl345Register_DataFormat_Range *range) {
  if (NULL == range)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(Adxl345Register_Address_dataFormat, &reg);
  *range = (enum Adxl345Register_DataFormat_Range)reg.asDataFormat.range;

  return 0;
}

int Adxl345_getScale(enum Adxl345Register_DataFormat_FullResBit *scale) {
  if (NULL == scale)
    return -EINVAL;

  union Adxl345Register reg = {0};
  readRegister(Adxl345Register_Address_dataFormat, &reg);
  *scale = (enum Adxl345Register_DataFormat_FullResBit)reg.asDataFormat.fullRes;

  return 0;
}

int Adxl345_setScale(uint8_t scale) {
  switch ((enum Adxl345Register_DataFormat_FullResBit)scale) {
  case Adxl345Register_DataFormat_FullResBit_10bit:
  case Adxl345Register_DataFormat_FullResBit_fullRes_4mg: {
    union Adxl345Register reg = {0};
    readRegister(Adxl345Register_Address_dataFormat, &reg);
    reg.asDataFormat.fullRes =
        (enum Adxl345Register_DataFormat_FullResBit)scale;
    writeRegister(Adxl345Register_Address_dataFormat, &reg);
  } break;

  default:
    return -EINVAL;
  }

  return 0;
}

void Adxl345_setPowerCtlStandby() {
  union Adxl345Register reg = {0};
  readRegister(Adxl345Register_Address_powerCtl, &reg);
  reg.asPowerControl.measure = Adxl345Register_PowerCtl_Measure_standby;
  writeRegister(Adxl345Register_Address_powerCtl, &reg);
}

void Adxl345_setPowerCtlMeasure() {
  union Adxl345Register reg = {0};
  readRegister(Adxl345Register_Address_powerCtl, &reg);
  reg.asPowerControl.measure = Adxl345Register_PowerCtl_Measure_measure;
  writeRegister(Adxl345Register_Address_powerCtl, &reg);
}

int Adxl345_getAcceleration(struct Adxl345_Acceleration *acc) {
  if (NULL == acc)
    return -EINVAL;

  union Adxl345TxFrame tx_frame = {.asAddress = Adxl345Register_Address_dataX0};
  union Adxl345RxFrame rx_frame = {0};
  transmitReceiveFrame(&tx_frame, &rx_frame, 6);
  *acc = rx_frame.asAcceleration;

  return 0;
}
