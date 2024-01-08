/**
 * \file adxl345_flags.h
 *
 * ADXL345 address and register flags definitions.
 */

#pragma once

/* ADXL345 Sensor Register Addresses -----------------------------------------*/

/**
 * ADXL345 register address listing.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G p.23
 */
enum Adxl345Flags_Address {
  Adxl345Flags_Address_devId = 0x00, // expected 0b11100101
  Adxl345Flags_Address_reserved_01 = 0x01,
  Adxl345Flags_Address_reserved_1C = 0x1C,
  Adxl345Flags_Address_thresTap = 0x1D,
  Adxl345Flags_Address_offsX = 0x1E,
  Adxl345Flags_Address_offsY = 0x1F,
  Adxl345Flags_Address_offsZ = 0x20,
  Adxl345Flags_Address_dur = 0x21,
  Adxl345Flags_Address_latent = 0x22,
  Adxl345Flags_Address_window = 0x23,
  Adxl345Flags_Address_thresAct = 0x24,
  Adxl345Flags_Address_thresInact = 0x25,
  Adxl345Flags_Address_timeInact = 0x26,
  Adxl345Flags_Address_actInactCtl = 0x27,
  Adxl345Flags_Address_thresFf = 0x28,
  Adxl345Flags_Address_timeFf = 0x29,
  Adxl345Flags_Address_tapAxes = 0x2A,
  Adxl345Flags_Address_actTapStatus = 0x2B,
  Adxl345Flags_Address_bwRate = 0x2C,
  Adxl345Flags_Address_powerCtl = 0x2D,
  Adxl345Flags_Address_intEnable = 0x2E,
  Adxl345Flags_Address_intMap = 0x2F,
  Adxl345Flags_Address_intSource = 0x30,
  Adxl345Flags_Address_dataFormat = 0x31,
  Adxl345Flags_Address_dataX0 = 0x32,
  Adxl345Flags_Address_dataX1 = 0x33,
  Adxl345Flags_Address_dataY0 = 0x34,
  Adxl345Flags_Address_dataY1 = 0x35,
  Adxl345Flags_Address_dataZ0 = 0x36,
  Adxl345Flags_Address_dataZ1 = 0x37,
  Adxl345Flags_Address_fifoCtl = 0x38,
  Adxl345Flags_Address_fifoStatus = 0x39,
};

/* ADXL345 Register Flags ----------------------------------------------------*/

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_PowerCtl_Wakeup {
  Adxl345Flags_PowerCtl_Wakeup_8Hz = 0b00,
  Adxl345Flags_PowerCtl_Wakeup_4Hz = 0b01,
  Adxl345Flags_PowerCtl_Wakeup_2Hz = 0b10,
  Adxl345Flags_PowerCtl_Wakeup_1Hz = 0b11,
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_PowerCtl_Sleep {
  Adxl345Flags_PowerCtl_Sleep_normalMode = 0,
  Adxl345Flags_PowerCtl_Sleep_sleepMode
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_PowerCtl_Measure {
  Adxl345Flags_PowerCtl_Measure_standby = 0,
  Adxl345Flags_PowerCtl_Measure_measure
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_PowerCtl_AutoSleep {
  Adxl345Flags_PowerCtl_AutoSleep_disabled = 0,
  Adxl345Flags_PowerCtl_AutoSleep_function
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_PowerCtl_Link {
  Adxl345Flags_PowerCtl_Link_concurrent = 0,
  Adxl345Flags_PowerCtl_Link_serial
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_Overrun {
  Adxl345Flags_IntEnable_Overrun_disable = 0,
  Adxl345Flags_IntEnable_Overrun_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_Watermark {
  Adxl345Flags_IntEnable_Watermark_disable = 0,
  Adxl345Flags_IntEnable_Watermark_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_FreeFall {
  Adxl345Flags_IntEnable_FreeFall_disable = 0,
  Adxl345Flags_IntEnable_FreeFall_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_Inactivity {
  Adxl345Flags_IntEnable_Inactivity_disable = 0,
  Adxl345Flags_IntEnable_Inactivity_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_Activity {
  Adxl345Flags_IntEnable_Activity_disable = 0,
  Adxl345Flags_IntEnable_Activity_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_DoubleTap {
  Adxl345Flags_IntEnable_DoubleTap_disable = 0,
  Adxl345Flags_IntEnable_DoubleTap_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_SingleTap {
  Adxl345Flags_IntEnable_SingleTap_disable = 0,
  Adxl345Flags_IntEnable_SingleTap_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntEnable_DataReady {
  Adxl345Flags_IntEnable_DataReady_disable = 0,
  Adxl345Flags_IntEnable_DataReady_enable
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_Overrun {
  Adxl345Flags_IntMap_Overrun_int1 = 0,
  Adxl345Flags_IntMap_Overrun_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_Watermark {
  Adxl345Flags_IntMap_Watermark_int1 = 0,
  Adxl345Flags_IntMap_Watermark_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_FreeFall {
  Adxl345Flags_IntMap_FreeFall_int1 = 0,
  Adxl345Flags_IntMap_FreeFall_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_Inactivity {
  Adxl345Flags_IntMap_Inactivity_int1 = 0,
  Adxl345Flags_IntMap_Inactivity_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_Activity {
  Adxl345Flags_IntMap_Activity_int1 = 0,
  Adxl345Flags_IntMap_Activity_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_DoubleTap {
  Adxl345Flags_IntMap_DoubleTap_int1 = 0,
  Adxl345Flags_IntMap_DoubleTap_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_SingleTap {
  Adxl345Flags_IntMap_SingleTap_int1 = 0,
  Adxl345Flags_IntMap_SingleTap_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_IntMap_DataReady {
  Adxl345Flags_IntMap_DataReady_int1 = 0,
  Adxl345Flags_IntMap_DataReady_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_SelfTest {
  Adxl345Flags_DataFormat_SelfTest_disableForce = 0,
  Adxl345Flags_DataFormat_SelfTest_enableForce
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_SpiBit {
  Adxl345Flags_DataFormat_SpiBit_3wire = 1,
  Adxl345Flags_DataFormat_SpiBit_4wire = 0
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_IntInvert {
  Adxl345Flags_DataFormat_IntInvert_activeHigh = 0,
  Adxl345Flags_DataFormat_IntInvert_activeLow
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_FullResBit {
  // full range 10bit: each range scales to 10 bit output
  Adxl345Flags_DataFormat_FullResBit_10bit = 0,
  // full resolution (16 bit): each range maintains 4mg/LSB
  Adxl345Flags_DataFormat_FullResBit_fullRes_4mg
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_Justify {
  Adxl345Flags_DataFormat_Justify_lsbRight = 0, ///< intuitive ordering
  Adxl345Flags_DataFormat_Justify_msbLeft       ///< left aligned
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_DataFormat_Range {
  Adxl345Flags_DataFormat_Range_2g = 0b00,
  Adxl345Flags_DataFormat_Range_4g = 0b01,
  Adxl345Flags_DataFormat_Range_8g = 0b10,
  Adxl345Flags_DataFormat_Range_16g = 0b11
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_BwRate_Rate {
  Adxl345Flags_BwRate_Rate_normalPowerOdr3200 = 0b1111,
  Adxl345Flags_BwRate_Rate_normalPowerOdr1600 = 0b1110,
  Adxl345Flags_BwRate_Rate_normalPowerOdr800 = 0b1101,
  Adxl345Flags_BwRate_Rate_normalPowerOdr400 = 0b1100,
  Adxl345Flags_BwRate_Rate_normalPowerOdr200 = 0b1011,
  Adxl345Flags_BwRate_Rate_normalPowerOdr100 = 0b1010,
  Adxl345Flags_BwRate_Rate_normalPowerOdr50 = 0b1001,
  Adxl345Flags_BwRate_Rate_normalPowerOdr25 = 0b1000,
  Adxl345Flags_BwRate_Rate_normalPowerOdr12_5 = 0b0111,
  Adxl345Flags_BwRate_Rate_normalPowerOdr6_25 = 0b0110,
  Adxl345Flags_BwRate_Rate_normalPowerOdr3_13 = 0b0101,
  Adxl345Flags_BwRate_Rate_normalPowerOdr1_56 = 0b0100,
  Adxl345Flags_BwRate_Rate_normalPowerOdr0_78 = 0b0011,
  Adxl345Flags_BwRate_Rate_normalPowerOdr0_39 = 0b0010,
  Adxl345Flags_BwRate_Rate_normalPowerOdr0_20 = 0b0001,
  Adxl345Flags_BwRate_Rate_normalPowerOdr0_10 = 0b0000,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr400 = 0b1100,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr200 = 0b1011,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr100 = 0b1010,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr50 = 0b1001,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr25 = 0b1000,
  Adxl345Flags_BwRate_Rate_reducedPowerOdr12_5 = 0b0111
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_BwRate_LowPower {
  Adxl345Flags_BwRate_LowPower_normal = 0,
  Adxl345Flags_BwRate_LowPower_reduced
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_FifoCtl_Trigger {
  Adxl345Flags_FifoCtl_Trigger_int1 = 0,
  Adxl345Flags_FifoCtl_Trigger_int2
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_FifoCtl_FifoMode {
  Adxl345Flags_FifoCtl_FifoMode_bypass = 0b00,
  Adxl345Flags_FifoCtl_FifoMode_fifo = 0b01,
  Adxl345Flags_FifoCtl_FifoMode_stream = 0b10,
  Adxl345Flags_FifoCtl_FifoMode_trigger = 0b11
};

/**
 * ADXL345 register flags.
 *
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum Adxl345Flags_FifoStatus_FifoTrig {
  Adxl345Flags_FifoStatus_FifoTrig_triggered = 0,
  Adxl345Flags_FifoStatus_FifoTrig_notTriggered
};
