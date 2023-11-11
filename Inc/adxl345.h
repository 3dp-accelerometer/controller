#pragma once

#include <assert.h>
#include <inttypes.h>

/* FiFo  Constants -----------------------------------------------------------*/

#define ADXL345_FIFO_ENTRIES 32 // 32 * (X, Y, Z); 2 bytes each coordinate
#define ADXL345_WATERMARK_LEVEL 24

static_assert(ADXL345_WATERMARK_LEVEL <= 32,
              "maximum allowed watermark level: 32");
static_assert(ADXL345_WATERMARK_LEVEL >= 0,
              "minimum allowed watermark level: 0");

/* TX/RX Constants -----------------------------------------------------------*/

enum Adxl345CS { Adxl345CS_modify = 0, Adxl345CS_untouched };
enum Adxl345RWFlags {
  Adxl345RWFlags_read = 0x80,
  Adxl345RWFlags_write = 0x00,
  Adxl345RWFlags_multiByte = 0x40,
  Adxl345RWFlags_singleByte = 0x00,
};

/* Register Addresses --------------------------------------------------------*/

enum Adxl345Register_Address {
  Adxl345Register_Address_devId = 0x00, // expected 0b11100101
  __Adxl345Register_Address_reserved_01 = 0x01,
  __Adxl345Register_Address_reserved_1C = 0x1C,
  Adxl345Register_Address_thresTap = 0x1D,
  Adxl345Register_Address_offsX = 0x1E,
  Adxl345Register_Address_offsY = 0x1F,
  Adxl345Register_Address_offsZ = 0x20,
  Adxl345Register_Address_dur = 0x21,
  Adxl345Register_Address_latent = 0x22,
  Adxl345Register_Address_window = 0x23,
  Adxl345Register_Address_thresAct = 0x24,
  Adxl345Register_Address_thresInact = 0x25,
  Adxl345Register_Address_timeInact = 0x26,
  Adxl345Register_Address_actInactCtl = 0x27,
  Adxl345Register_Address_thresFf = 0x28,
  Adxl345Register_Address_timeFf = 0x29,
  Adxl345Register_Address_tapAxes = 0x2A,
  Adxl345Register_Address_actTapStatus = 0x2B,
  Adxl345Register_Address_bwRate = 0x2C,
  Adxl345Register_Address_powerCtl = 0x2D,
  Adxl345Register_Address_intEnable = 0x2E,
  Adxl345Register_Address_intMap = 0x2F,
  Adxl345Register_Address_intSource = 0x30,
  Adxl345Register_Address_dataFormat = 0x31,
  Adxl345Register_Address_dataX0 = 0x32,
  Adxl345Register_Address_dataX1 = 0x33,
  Adxl345Register_Address_dataY0 = 0x34,
  Adxl345Register_Address_dataY1 = 0x35,
  Adxl345Register_Address_dataZ0 = 0x36,
  Adxl345Register_Address_dataZ1 = 0x37,
  Adxl345Register_Address_fifoCtl = 0x38,
  Adxl345Register_Address_fifoStatus = 0x39,
};

/* Register Flags ------------------------------------------------------------*/

enum Adxl345Register_PowerCtl_Wakeup {
  Adxl345Register_PowerCtl_Wakeup_8Hz = 0b00,
  Adxl345Register_PowerCtl_Wakeup_4Hz = 0b01,
  Adxl345Register_PowerCtl_Wakeup_2Hz = 0b10,
  Adxl345Register_PowerCtl_Wakeup_1Hz = 0b11,
};

enum Adxl345Register_PowerCtl_Sleep {
  Adxl345Register_PowerCtl_Sleep_normalMode = 0,
  Adxl345Register_PowerCtl_Sleep_sleepMode
};

enum Adxl345Register_PowerCtl_Measure {
  Adxl345Register_PowerCtl_Measure_standby = 0,
  Adxl345Register_PowerCtl_Measure_measure
};

enum Adxl345Register_PowerCtl_AutoSleep {
  Adxl345Register_PowerCtl_AutoSleep_disabled = 0,
  Adxl345Register_PowerCtl_AutoSleep_function
};

enum Adxl345Register_PowerCtl_Link {
  Adxl345Register_PowerCtl_Link_concurrent = 0,
  Adxl345Register_PowerCtl_Link_serial
};

enum Adxl345Register_IntEnable_Overrun {
  Adxl345Register_IntEnable_Overrun_disable = 0,
  Adxl345Register_IntEnable_Overrun_enable
};

enum Adxl345Register_IntEnable_Watermark {
  Adxl345Register_IntEnable_Watermark_disable = 0,
  Adxl345Register_IntEnable_Watermark_enable
};

enum Adxl345Register_IntEnable_FreeFall {
  Adxl345Register_IntEnable_FreeFall_disable = 0,
  Adxl345Register_IntEnable_FreeFall_enable
};

enum Adxl345Register_IntEnable_Inactivity {
  Adxl345Register_IntEnable_Inactivity_disable = 0,
  Adxl345Register_IntEnable_Inactivity_enable
};

enum Adxl345Register_IntEnable_Activity {
  Adxl345Register_IntEnable_Activity_disable = 0,
  Adxl345Register_IntEnable_Activity_enable
};

enum Adxl345Register_IntEnable_DoubleTap {
  Adxl345Register_IntEnable_DoubleTap_disable = 0,
  Adxl345Register_IntEnable_DoubleTap_enable
};

enum Adxl345Register_IntEnable_SingleTap {
  Adxl345Register_IntEnable_SingleTap_disable = 0,
  Adxl345Register_IntEnable_SingleTap_enable
};

enum Adxl345Register_IntEnable_DataReady {
  Adxl345Register_IntEnable_DataReady_disable = 0,
  Adxl345Register_IntEnable_DataReady_enable
};

enum Adxl345Register_IntMap_Overrun {
  Adxl345Register_IntMap_Overrun_int1 = 0,
  Adxl345Register_IntMap_Overrun_int2
};

enum Adxl345Register_IntMap_Watermark {
  Adxl345Register_IntMap_Watermark_int1 = 0,
  Adxl345Register_IntMap_Watermark_int2
};

enum Adxl345Register_IntMap_FreeFall {
  Adxl345Register_IntMap_FreeFall_int1 = 0,
  Adxl345Register_IntMap_FreeFall_int2
};

enum Adxl345Register_IntMap_Inactivity {
  Adxl345Register_IntMap_Inactivity_int1 = 0,
  Adxl345Register_IntMap_Inactivity_int2
};

enum Adxl345Register_IntMap_Activity {
  Adxl345Register_IntMap_Activity_int1 = 0,
  Adxl345Register_IntMap_Activity_int2
};

enum Adxl345Register_IntMap_DoubleTap {
  Adxl345Register_IntMap_DoubleTap_int1 = 0,
  Adxl345Register_IntMap_DoubleTap_int2
};

enum Adxl345Register_IntMap_SingleTap {
  Adxl345Register_IntMap_SingleTap_int1 = 0,
  Adxl345Register_IntMap_SingleTap_int2
};

enum Adxl345Register_IntMap_DataReady {
  Adxl345Register_IntMap_DataReady_int1 = 0,
  Adxl345Register_IntMap_DataReady_int2
};

enum Adxl345Register_DataFormat_SelfTest {
  Adxl345Register_DataFormat_SelfTest_disableForce = 0,
  Adxl345Register_DataFormat_SelfTest_enableForce
};

enum Adxl345Register_DataFormat_SpiBit {
  Adxl345Register_DataFormat_SpiBit_3wire = 1,
  Adxl345Register_DataFormat_SpiBit_4wire = 0
};

enum Adxl345Register_DataFormat_IntInvert {
  Adxl345Register_DataFormat_IntInvert_activeHigh = 0,
  Adxl345Register_DataFormat_IntInvert_activeLow
};

enum Adxl345Register_DataFormat_FullResBit {
  // full range 10bit: each range scales to 10 bit output
  Adxl345Register_DataFormat_FullResBit_10bit = 0,
  // full resolution (16 bit): each range maintains 4mg/LSB
  Adxl345Register_DataFormat_FullResBit_fullRes_4mg
};

enum Adxl345Register_DataFormat_Justify {
  Adxl345Register_DataFormat_Justify_lsbRight = 0,
  Adxl345Register_DataFormat_Justify_msbLeft
};

enum Adxl345Register_DataFormat_Range {
  Adxl345Register_DataFormat_Range_2g = 0b00,
  Adxl345Register_DataFormat_Range_4g = 0b01,
  Adxl345Register_DataFormat_Range_8g = 0b10,
  Adxl345Register_DataFormat_Range_16g = 0b11
};

enum Adxl345Register_BwRate_Rate {
  Adxl345Register_BwRate_Rate_normalPowerOdr3200 = 0b1111,
  Adxl345Register_BwRate_Rate_normalPowerOdr1600 = 0b1110,
  Adxl345Register_BwRate_Rate_normalPowerOdr800 = 0b1101,
  Adxl345Register_BwRate_Rate_normalPowerOdr400 = 0b1100,
  Adxl345Register_BwRate_Rate_normalPowerOdr200 = 0b1011,
  Adxl345Register_BwRate_Rate_normalPowerOdr100 = 0b1010,
  Adxl345Register_BwRate_Rate_normalPowerOdr50 = 0b1001,
  Adxl345Register_BwRate_Rate_normalPowerOdr25 = 0b1000,
  Adxl345Register_BwRate_Rate_normalPowerOdr12_5 = 0b0111,
  Adxl345Register_BwRate_Rate_normalPowerOdr6_25 = 0b0110,
  Adxl345Register_BwRate_Rate_normalPowerOdr3_13 = 0b0101,
  Adxl345Register_BwRate_Rate_normalPowerOdr1_56 = 0b0100,
  Adxl345Register_BwRate_Rate_normalPowerOdr0_78 = 0b0011,
  Adxl345Register_BwRate_Rate_normalPowerOdr0_39 = 0b0010,
  Adxl345Register_BwRate_Rate_normalPowerOdr0_20 = 0b0001,
  Adxl345Register_BwRate_Rate_normalPowerOdr0_10 = 0b0000,
  Adxl345Register_BwRate_Rate_reducedPowerOdr400 = 0b1100,
  Adxl345Register_BwRate_Rate_reducedPowerOdr200 = 0b1011,
  Adxl345Register_BwRate_Rate_reducedPowerOdr100 = 0b1010,
  Adxl345Register_BwRate_Rate_reducedPowerOdr50 = 0b1001,
  Adxl345Register_BwRate_Rate_reducedPowerOdr25 = 0b1000,
  Adxl345Register_BwRate_Rate_reducedPowerOdr12_5 = 0b0111
};

enum Adxl345Register_BwRate_LowPower {
  Adxl345Register_BwRate_LowPower_normal = 0,
  Adxl345Register_BwRate_LowPower_reduced
};

enum Adxl345Register_FifoCtl_Trigger {
  Adxl345Register_FifoCtl_Trigger_int1 = 0,
  Adxl345Register_FifoCtl_Trigger_int2
};

enum Adxl345Register_FifoCtl_FifoMode {
  Adxl345Register_FifoCtl_FifoMode_bypass = 0b00,
  Adxl345Register_FifoCtl_FifoMode_fifo = 0b01,
  Adxl345Register_FifoCtl_FifoMode_stream = 0b10,
  Adxl345Register_FifoCtl_FifoMode_trigger = 0b11
};

enum Adxl345Register_FifoStatus_FifoTrig {
  Adxl345Register_FifoStatus_FifoTrig_triggered = 0,
  Adxl345Register_FifoStatus_FifoTrig_notTriggered
};

/* Registers -----------------------------------------------------------------*/

struct Adxl345Register_BwRate {
  uint8_t rate : 4;
  uint8_t lowPower : 1;
  uint8_t _zeroD5 : 1;
  uint8_t _zeroD6 : 1;
  uint8_t _zeroD7 : 1;
} __attribute__((packed));

struct Adxl345Register_PowerCtl {
  uint8_t wakeup : 2;
  uint8_t sleep : 1;
  uint8_t measure : 1;
  uint8_t autoSleep : 1;
  uint8_t link : 1;
  uint8_t _zeroD6 : 1;
  uint8_t _zeroD7 : 1;
} __attribute__((packed));

struct Adxl345Register_IntEnable {
  uint8_t overrun : 1;
  uint8_t watermark : 1;
  uint8_t freeFall : 1;
  uint8_t inactivity : 1;
  uint8_t activity : 1;
  uint8_t doubleTap : 1;
  uint8_t singleTap : 1;
  uint8_t dataReady : 1;
} __attribute__((packed));

struct Adxl345Register_IntMap {
  uint8_t overrun : 1;
  uint8_t watermark : 1;
  uint8_t freeFall : 1;
  uint8_t inactivity : 1;
  uint8_t activity : 1;
  uint8_t doubleTap : 1;
  uint8_t singleTap : 1;
  uint8_t dataReady : 1;
} __attribute__((packed));

struct Adxl345Register_DataFormat {
  uint8_t range : 2;
  uint8_t justify : 1;
  uint8_t fullRes : 1;
  uint8_t _zeroD4 : 1;
  uint8_t intInvert : 1;
  uint8_t spi : 1;
  uint8_t selfTest : 1;
} __attribute__((packed));

struct Adxl345Register_FifoCtl {
  uint8_t samples : 5; // see Table 38. Samples Bits Functions
  uint8_t trigger : 1;
  uint8_t fifoMode : 2;
} __attribute__((packed));

struct Adxl345Register_FifoStatus {
  uint8_t entries : 6;
  uint8_t _zeroD6 : 1;
  uint8_t fifoTrig : 1;
} __attribute__((packed));

/* Device Register Listing ---------------------------------------------------*/

struct TwoBytes {
  uint8_t byte1;
  uint8_t byte2;
} __attribute__((packed));

union Adxl345Register {
  struct Adxl345Register_DataFormat asDataFormat;
  struct Adxl345Register_BwRate asBwRate;
  struct Adxl345Register_PowerCtl asPowerControl;
  struct Adxl345Register_IntEnable asIntEnable;
  struct Adxl345Register_IntMap asIntMap;
  struct Adxl345Register_FifoCtl asFifoCtl;
  struct Adxl345Register_FifoStatus asFifoStatus;
} __attribute__((packed));

/* TX Frame ------------------------------------------------------------------*/

struct Adxl345DataPaddedRegister {
  uint8_t _padding8;
  union Adxl345Register asRegister;
} __attribute__((packed));

union Adxl345TxFrame {
  uint16_t asWord;
  struct TwoBytes asBytes;
  uint8_t asAddress;
  struct Adxl345DataPaddedRegister asPaddedRegister;
} __attribute__((packed));

/* RX Frame ------------------------------------------------------------------*/

struct Adxl345_Acceleration {
  int16_t x;
  int16_t y;
  int16_t z;
} __attribute__((packed));

union Adxl345RxFrame {
  union Adxl345Register asRegister;
  struct Adxl345_Acceleration asAcceleration;
  struct TwoBytes asBytes;
  uint16_t asWord;
} __attribute__((packed));

/* ---------------------------------------------------------------------------*/

int Adxl345_init();

int Adxl345_getOutputDataRate(enum Adxl345Register_BwRate_Rate *rate);
int Adxl345_setOutputDataRate(uint8_t rate);

int Adxl345_getRange(enum Adxl345Register_DataFormat_Range *range);
int Adxl345_setRange(uint8_t range);

int Adxl345_getScale(enum Adxl345Register_DataFormat_FullResBit *scale);
int Adxl345_setScale(uint8_t scale);

void Adxl345_setPowerCtlStandby();
void Adxl345_setPowerCtlMeasure();

int Adxl345_getAcceleration(struct Adxl345_Acceleration *acc);
