/**
 * \file adxl345_register.h
 *
 * ADXL345 register definitions.
 */

#pragma once

#include <inttypes.h>

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
  uint8_t samples : 5;  ///< see Table 38. Samples Bits Functions \see
                        ///< Adxl345Register_FifoCtl_Samples
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
 * Union for convenient type-casting in between raw data and respective register
 * type without c-style cast.
 *
 * Provides convenient and readable "casting".
 */
union Adxl345Register {
  struct Adxl345Register_DataFormat
      asDataFormat; ///< cast to Adxl345Register_DataFormat
  struct Adxl345Register_BwRate asBwRate; ///< cast to Adxl345Register_BwRate
  struct Adxl345Register_PowerCtl
      asPowerControl; ///< cast to Adxl345Register_PowerCtl
  struct Adxl345Register_IntEnable
      asIntEnable;                        ///< cast to Adxl345Register_IntEnable
  struct Adxl345Register_IntMap asIntMap; ///< cast to Adxl345Register_IntMap
  struct Adxl345Register_FifoCtl asFifoCtl; ///< cast to Adxl345Register_FifoCtl
  struct Adxl345Register_FifoStatus
      asFifoStatus; ///< cast to Adxl345Register_FifoStatus
} __attribute__((packed));
