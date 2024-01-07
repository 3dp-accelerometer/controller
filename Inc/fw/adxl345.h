/**
 * \file adxl345.h
 *
 * Accelerometer API for manipulating an ADXL345 sensor.
 */
#pragma once

#include <assert.h>
#include <inttypes.h>

/* FiFo  Constants -----------------------------------------------------------*/

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
              "maximum allowed watermark level: 32");
static_assert(ADXL345_WATERMARK_LEVEL >= 0,
              "minimum allowed watermark level: 0");

/* TX/RX SPI Constants
 * -----------------------------------------------------------*/

/**
 * Chip Select flags for SPI communication.
 */
enum Adxl345CS { Adxl345CS_modify = 0, Adxl345CS_untouched };

/**
 * Read/write flags for SPI communication
 */
enum Adxl345RWFlags {
  Adxl345RWFlags_read = 0x80,
  Adxl345RWFlags_write = 0x00,
  Adxl345RWFlags_multiByte = 0x40,
  Adxl345RWFlags_singleByte = 0x00,
};

/* ADXL345 Sensor Register Addresses -----------------------------------------*/

/**
 * ADXL345 register address listing.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G p.23
 */
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

/* ADXL345 Register Flags ----------------------------------------------------*/

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_PowerCtl_Wakeup {
  Adxl345Register_PowerCtl_Wakeup_8Hz = 0b00,
  Adxl345Register_PowerCtl_Wakeup_4Hz = 0b01,
  Adxl345Register_PowerCtl_Wakeup_2Hz = 0b10,
  Adxl345Register_PowerCtl_Wakeup_1Hz = 0b11,
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_PowerCtl_Sleep {
  Adxl345Register_PowerCtl_Sleep_normalMode = 0,
  Adxl345Register_PowerCtl_Sleep_sleepMode
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_PowerCtl_Measure {
  Adxl345Register_PowerCtl_Measure_standby = 0,
  Adxl345Register_PowerCtl_Measure_measure
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_PowerCtl_AutoSleep {
  Adxl345Register_PowerCtl_AutoSleep_disabled = 0,
  Adxl345Register_PowerCtl_AutoSleep_function
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_PowerCtl_Link {
  Adxl345Register_PowerCtl_Link_concurrent = 0,
  Adxl345Register_PowerCtl_Link_serial
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_Overrun {
  Adxl345Register_IntEnable_Overrun_disable = 0,
  Adxl345Register_IntEnable_Overrun_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_Watermark {
  Adxl345Register_IntEnable_Watermark_disable = 0,
  Adxl345Register_IntEnable_Watermark_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_FreeFall {
  Adxl345Register_IntEnable_FreeFall_disable = 0,
  Adxl345Register_IntEnable_FreeFall_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_Inactivity {
  Adxl345Register_IntEnable_Inactivity_disable = 0,
  Adxl345Register_IntEnable_Inactivity_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_Activity {
  Adxl345Register_IntEnable_Activity_disable = 0,
  Adxl345Register_IntEnable_Activity_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_DoubleTap {
  Adxl345Register_IntEnable_DoubleTap_disable = 0,
  Adxl345Register_IntEnable_DoubleTap_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_SingleTap {
  Adxl345Register_IntEnable_SingleTap_disable = 0,
  Adxl345Register_IntEnable_SingleTap_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntEnable_DataReady {
  Adxl345Register_IntEnable_DataReady_disable = 0,
  Adxl345Register_IntEnable_DataReady_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_Overrun {
  Adxl345Register_IntMap_Overrun_int1 = 0,
  Adxl345Register_IntMap_Overrun_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_Watermark {
  Adxl345Register_IntMap_Watermark_int1 = 0,
  Adxl345Register_IntMap_Watermark_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_FreeFall {
  Adxl345Register_IntMap_FreeFall_int1 = 0,
  Adxl345Register_IntMap_FreeFall_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_Inactivity {
  Adxl345Register_IntMap_Inactivity_int1 = 0,
  Adxl345Register_IntMap_Inactivity_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_Activity {
  Adxl345Register_IntMap_Activity_int1 = 0,
  Adxl345Register_IntMap_Activity_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_DoubleTap {
  Adxl345Register_IntMap_DoubleTap_int1 = 0,
  Adxl345Register_IntMap_DoubleTap_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_SingleTap {
  Adxl345Register_IntMap_SingleTap_int1 = 0,
  Adxl345Register_IntMap_SingleTap_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_IntMap_DataReady {
  Adxl345Register_IntMap_DataReady_int1 = 0,
  Adxl345Register_IntMap_DataReady_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_SelfTest {
  Adxl345Register_DataFormat_SelfTest_disableForce = 0,
  Adxl345Register_DataFormat_SelfTest_enableForce
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_SpiBit {
  Adxl345Register_DataFormat_SpiBit_3wire = 1,
  Adxl345Register_DataFormat_SpiBit_4wire = 0
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_IntInvert {
  Adxl345Register_DataFormat_IntInvert_activeHigh = 0,
  Adxl345Register_DataFormat_IntInvert_activeLow
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_FullResBit {
  // full range 10bit: each range scales to 10 bit output
  Adxl345Register_DataFormat_FullResBit_10bit = 0,
  // full resolution (16 bit): each range maintains 4mg/LSB
  Adxl345Register_DataFormat_FullResBit_fullRes_4mg
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_Justify {
  Adxl345Register_DataFormat_Justify_lsbRight = 0, ///< intuitive ordering
  Adxl345Register_DataFormat_Justify_msbLeft       ///< left aligned
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_DataFormat_Range {
  Adxl345Register_DataFormat_Range_2g = 0b00,
  Adxl345Register_DataFormat_Range_4g = 0b01,
  Adxl345Register_DataFormat_Range_8g = 0b10,
  Adxl345Register_DataFormat_Range_16g = 0b11
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
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

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_BwRate_LowPower {
  Adxl345Register_BwRate_LowPower_normal = 0,
  Adxl345Register_BwRate_LowPower_reduced
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_FifoCtl_Trigger {
  Adxl345Register_FifoCtl_Trigger_int1 = 0,
  Adxl345Register_FifoCtl_Trigger_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_FifoCtl_FifoMode {
  Adxl345Register_FifoCtl_FifoMode_bypass = 0b00,
  Adxl345Register_FifoCtl_FifoMode_fifo = 0b01,
  Adxl345Register_FifoCtl_FifoMode_stream = 0b10,
  Adxl345Register_FifoCtl_FifoMode_trigger = 0b11
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Register_FifoStatus_FifoTrig {
  Adxl345Register_FifoStatus_FifoTrig_triggered = 0,
  Adxl345Register_FifoStatus_FifoTrig_notTriggered
};

/* ADXL345 Sensor Registers --------------------------------------------------*/

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_BwRate {
  uint8_t rate : 4;     ///< \see Adxl345Register_BwRate_Rate
  uint8_t lowPower : 1; ///< \see Adxl345Register_BwRate_LowPower
  uint8_t _zeroD5 : 1;  ///< reserved
  uint8_t _zeroD6 : 1;  ///< reserved
  uint8_t _zeroD7 : 1;  ///< reserved
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_PowerCtl {
  uint8_t wakeup : 2;    ///< \see Adxl345Register_PowerCtl_Wakeup
  uint8_t sleep : 1;     ///< \see Adxl345Register_PowerCtl_Sleep
  uint8_t measure : 1;   ///< \see Adxl345Register_PowerCtl_Measure
  uint8_t autoSleep : 1; ///< \see Adxl345Register_PowerCtl_AutoSleep
  uint8_t link : 1;      ///< \see Adxl345Register_PowerCtl_Link
  uint8_t _zeroD6 : 1;   ///< reserved
  uint8_t _zeroD7 : 1;   ///< reserved
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_IntEnable {
  uint8_t overrun : 1;    ///< \see Adxl345Register_IntEnable_Overrun
  uint8_t watermark : 1;  ///< \see Adxl345Register_IntEnable_Watermark
  uint8_t freeFall : 1;   ///< \see Adxl345Register_IntEnable_FreeFall
  uint8_t inactivity : 1; ///< \see Adxl345Register_IntEnable_Inactivity
  uint8_t activity : 1;   ///< \see Adxl345Register_IntEnable_Activity
  uint8_t doubleTap : 1;  ///< \see Adxl345Register_IntEnable_DoubleTap
  uint8_t singleTap : 1;  ///< \see Adxl345Register_IntEnable_SingleTap
  uint8_t dataReady : 1;  ///< \see Adxl345Register_IntEnable_DataReady
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_IntMap {
  uint8_t overrun : 1;    ///< \see Adxl345Register_IntMap_Overrun
  uint8_t watermark : 1;  ///< \see Adxl345Register_IntMap_Watermark
  uint8_t freeFall : 1;   ///< \see Adxl345Register_IntMap_FreeFall
  uint8_t inactivity : 1; ///< \see Adxl345Register_IntMap_Inactivity
  uint8_t activity : 1;   ///< \see Adxl345Register_IntMap_Activity
  uint8_t doubleTap : 1;  ///< \see Adxl345Register_IntMap_DoubleTap
  uint8_t singleTap : 1;  ///< \see Adxl345Register_IntMap_SingleTap
  uint8_t dataReady : 1;  ///< \see Adxl345Register_IntMap_DataReady
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_DataFormat {
  uint8_t range : 2;     ///< \see Adxl345Register_DataFormat_Range
  uint8_t justify : 1;   ///< \see Adxl345Register_DataFormat_Justify
  uint8_t fullRes : 1;   ///< \see Adxl345Register_DataFormat_FullRes
  uint8_t _zeroD4 : 1;   ///< reserved
  uint8_t intInvert : 1; ///< \see Adxl345Register_DataFormat_IntInvert
  uint8_t spi : 1;       ///< \see Adxl345Register_DataFormat_Spi
  uint8_t selfTest : 1;  ///< \see Adxl345Register_DataFormat_SelfTest
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_FifoCtl {
  uint8_t samples : 5;  ///< see Table 38. Samples Bits Functions \see Adxl345Register_FifoCtl_Samples
  uint8_t trigger : 1;  ///< \see Adxl345Register_FifoCtl_Trigger
  uint8_t fifoMode : 2; ///< \see Adxl345Register_FifoCtl_FifoMode
} __attribute__((packed));

/**
 * ADXL345 register.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
struct Adxl345Register_FifoStatus {
  uint8_t entries : 6;  ///< \see Adxl345Register_FifoStatus_entries
  uint8_t _zeroD6 : 1;  ///< reserved
  uint8_t fifoTrig : 1; ///< \see Adxl345Register_FifoStatus_fifoTrig
} __attribute__((packed));

/* ADXL345 Sensor Device Register Listing ------------------------------------*/

/**
 * 2-Bytes struct for convenient byte access.
 *
 * Provides convenient and readable access to underlying bytes.
 */
struct TwoBytes {
  uint8_t byte1;
  uint8_t byte2;
} __attribute__((packed));

/**
 * Union for convenient type-casting in between raw data and respective register
 * type without c-style cast.
 *
 * Provides convenient and readable "casting".
 */
union Adxl345Register {
  struct Adxl345Register_DataFormat asDataFormat; ///< cast to Adxl345Register_DataFormat
  struct Adxl345Register_BwRate asBwRate;         ///< cast to Adxl345Register_BwRate
  struct Adxl345Register_PowerCtl asPowerControl; ///< cast to Adxl345Register_PowerCtl
  struct Adxl345Register_IntEnable asIntEnable;   ///< cast to Adxl345Register_IntEnable
  struct Adxl345Register_IntMap asIntMap;         ///< cast to Adxl345Register_IntMap
  struct Adxl345Register_FifoCtl asFifoCtl;       ///< cast to Adxl345Register_FifoCtl
  struct Adxl345Register_FifoStatus asFifoStatus; ///< cast to Adxl345Register_FifoStatus
} __attribute__((packed));

/* TX Frame ------------------------------------------------------------------*/

/**
 * Struct for convenient data access of padded register without bit-shift magic.
 */
struct Adxl345DataPaddedRegister {
  uint8_t _padding8;                ///< reserved
  union Adxl345Register asRegister; ///< cast to Adxl345Register
} __attribute__((packed));

/**
 * Union for convenient data access of words, bytes and padded registers
 * without bit-shift magic.
 */
union Adxl345TxFrame {
  uint16_t asWord;                                   ///< cast to uint16_t
  struct TwoBytes asBytes;                           ///< cast to TwoBytes
  uint8_t asAddress;                                 ///< cast to uint8_t
  struct Adxl345DataPaddedRegister asPaddedRegister; ///< cast to Adxl345DataPaddedRegister
} __attribute__((packed));

/* RX Frame ------------------------------------------------------------------*/

/**
 * Data acceleration struct with same byte layout as reports by ADXL345.
 */
struct Adxl345_Acceleration {
  int16_t x; ///< x-axis acceleration as seen from the sensor's perspective
  int16_t y; ///< y-axis acceleration as seen from the sensor's perspective
  int16_t z; ///< z-axis acceleration as seen from the sensor's perspective
} __attribute__((packed));

/**
 * Union for convenient type-casting in between raw data, bytes, word, register
 * and payload types without c-style cast.
 */
union Adxl345RxFrame {
  union Adxl345Register asRegister;           ///< cast to Adxl345Register
  struct Adxl345_Acceleration asAcceleration; ///< cast to Adxl345_Acceleration
  struct TwoBytes asBytes;                    ///< cast to TwoBytes
  uint16_t asWord;                            ///< cast to uint16_t
} __attribute__((packed));

/* ADXL345 Sensor API --------------------------------------------------------*/

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
int Adxl345_getAcceleration(struct Adxl345_Acceleration *acc);
