#include "adxl345.h"
#include "gpio.h"
#include "spi.h"
#include "usbd_cdc_if.h"
#include <stdio.h>
#include <string.h>

static void ncs_set() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_RESET);
}

static void ncs_clear() {
  HAL_GPIO_WritePin(SPI1_SS_GPIO_Port, SPI1_SS_Pin, GPIO_PIN_SET);
}

static void transmit_frame(union Adxl345TxFrame *frame, uint8_t num_bytes,
                          enum Adxl345CS apply_cs,
                          enum Adxl345RWFlags rw_flag) {
  frame->asAddress |= rw_flag;

  if (Adxl345CS_modify == apply_cs) {
    ncs_set();
    HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, num_bytes, 10);
    ncs_clear();
  } else {
    HAL_SPI_Transmit(&hspi1, (uint8_t *)frame, num_bytes, 10);
  }
}

static void receive_frame(union Adxl345RxFrame *frame, uint8_t num_bytes,
                         enum Adxl345CS apply_cs) {

  if (Adxl345CS_modify == apply_cs) {
    ncs_set();
    HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10);
    ncs_clear();
  } else {
    HAL_SPI_Receive(&hspi1, (uint8_t *)frame, num_bytes, 10);
  }
}

static int transmit_receive(union Adxl345TxFrame *tx_frame,
                            union Adxl345RxFrame *rx_frame,
                            uint8_t num_bytes_receive) {
  const uint8_t multiByte =
      num_bytes_receive > 1 ? Adxl345RWFlags_multiByte : 0;
  ncs_set();
  transmit_frame(tx_frame, 1, Adxl345CS_untouched,
                 Adxl345RWFlags_read | multiByte);
  receive_frame(rx_frame, num_bytes_receive, Adxl345CS_untouched);
  ncs_clear();

  return 0;
}

int Adxl345_init() {

  { // data format
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asDataFormat = {
            .selfTest = Adxl345Register_DataFormat_SelfTest_disable,
            .spi = Adxl345Register_DataFormat_SpiBit_4wire,
            .intInvert = Adxl345Register_DataFormat_IntInvert_activeHigh,
            ._zeroD4 = 0,
            .fullRes = Adxl345Register_DataFormat_FullResBit_10bit,
            .justify = Adxl345Register_DataFormat_Justify_msb,
            .range = Adxl345Register_DataFormat_Range_2g}};
    tx_frame.asAddress = Adxl345Register_Address_dataFormat;
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // bandwidth rate
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asBwRate = {
            .rate = Adxl345Register_BwRate_Rate_normalPowerOdr200,
            .lowPower = Adxl345Register_BwRate_LowPower_normal,
            ._zeroD5 = 0,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Register_Address_bwRate;
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // fifo control
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asFifoCtl = {
            .samples = 24,
            .trigger = Adxl345Register_FifoCtl_Trigger_int1,
            //.fifoMode = Adxl345Register_FifoCtl_FifoMode_bypass
            .fifoMode = Adxl345Register_FifoCtl_FifoMode_fifo}};
    tx_frame.asAddress = Adxl345Register_Address_fifoCtl;
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // power control
    union Adxl345TxFrame tx_frame = {
        .asPaddedRegister.asRegister.asPowerControl = {
            .wakeup = Adxl345Register_PowerCtl_Wakeup_8Hz,
            .sleep = Adxl345Register_PowerCtl_Sleep_normalMode,
            .measure = Adxl345Register_PowerCtl_Measure_measure,
            .autoSleep = Adxl345Register_PowerCtl_AutoSleep_disabled,
            .link = Adxl345Register_PowerCtl_Link_concurrent,
            ._zeroD6 = 0,
            ._zeroD7 = 0}};
    tx_frame.asAddress = Adxl345Register_Address_powerCtl;
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // interrupt enable
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
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  { // interrupt map
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
    transmit_frame(&tx_frame, 2, Adxl345CS_modify, Adxl345RWFlags_write);
  }

  return 0;
}

int Adxl345_checkDevId() {
  union Adxl345TxFrame tx_frame = {.asAddress = Adxl345Register_Address_devId};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "devId=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkBwRate() {
  union Adxl345TxFrame tx_frame = {.asAddress = Adxl345Register_Address_bwRate};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "bwRate=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkAcceleration() {
  union Adxl345TxFrame tx_frame = {.asAddress = Adxl345Register_Address_dataX0};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 6);
  char str[32];
  sprintf(str, "x=%d y=%d z=%d\r\n", rx_frame.asAcceleration.x,
          rx_frame.asAcceleration.y, rx_frame.asAcceleration.z);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkPowerCtl() {
  union Adxl345TxFrame tx_frame = {.asAddress = Adxl345Register_Address_bwRate};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "powerCtl=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkDataFormat() {
  union Adxl345TxFrame tx_frame = {.asAddress =
                                       Adxl345Register_Address_dataFormat};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "dataFormat=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkFifoCtl() {
  union Adxl345TxFrame tx_frame = {.asAddress =
                                       Adxl345Register_Address_fifoCtl};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "fifoCtl=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}

int Adxl345_checkFifoStatus() {
  union Adxl345TxFrame tx_frame = {.asAddress =
                                       Adxl345Register_Address_fifoStatus};
  union Adxl345RxFrame rx_frame = {0};

  transmit_receive(&tx_frame, &rx_frame, 1);
  char str[24];
  sprintf(str, "fifoStatus=%d\r\n", rx_frame.asBytes.byte1);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return 0;
}
