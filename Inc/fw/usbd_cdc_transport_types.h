/**
 * \file usbd_cdc_transport_types.h
 *
 * Types transferred from/to the IN USB endpoint (host).
 */

#pragma once
#include <inttypes.h>

/*----------------------------------------------------------------------------*/

struct Adxl345_Acceleration;

/*----------------------------------------------------------------------------*/

/**
 * Computes size of package, namely size of header type + size of package type.
 */
#define SIZEOF_HEADER_INCL_PAYLOAD(packageType)                                \
  (sizeof(struct Transport_Header) + sizeof(packageType))

/* Transport Header ----------------------------------------------------------*/

/**
 * TX/RX package type ID (header ID) listing.
 *
 * Each package's header ID must be one of these.
 */
enum Transport_HeaderId {
  Transport_HeaderId_Rx_SetOutputDataRate = 1,
  Transport_HeaderId_Rx_GetOutputDataRate,
  Transport_HeaderId_Rx_SetRange,
  Transport_HeaderId_Rx_GetRange,
  Transport_HeaderId_Rx_SetScale,
  Transport_HeaderId_Rx_GetScale,
  Transport_HeaderId_Rx_GetDeviceSetup,
  Transport_HeaderId_Rx_GetFirmwareVersion,

  Transport_HeaderId_Rx_DeviceReboot = 17,
  Transport_HeaderId_Rx_SamplingStart,
  Transport_HeaderId_Rx_SamplingStop,

  Transport_HeaderId_Tx_OutputDataRate = 25,
  Transport_HeaderId_Tx_Range,
  Transport_HeaderId_Tx_Scale,
  Transport_HeaderId_Tx_DeviceSetup,
  Transport_HeaderId_Tx_FirmwareVersion,

  Transport_HeaderId_Tx_FifoOverflow = 33,
  Transport_HeaderId_Tx_SamplingStarted,
  Transport_HeaderId_Tx_SamplingFinished,
  Transport_HeaderId_Tx_SamplingStopped,
  Transport_HeaderId_Tx_SamplingAborted,
  Transport_HeaderId_Tx_Acceleration,
};

/**
 * TX/RX package header.
 */
struct Transport_Header {
  enum Transport_HeaderId id;
} __attribute__((packed));

/* RX ------------------------------------------------------------------------*/

/**
 * Output data rate flags as described in data sheet.
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum TransportRx_SetOutputDataRate_Rate {
  TransportRx_SetOutputDataRate_Rate3200 = 0b1111,
  TransportRx_SetOutputDataRate_Rate1600 = 0b1110,
  TransportRx_SetOutputDataRate_Rate_800 = 0b1101,
  TransportRx_SetOutputDataRate_Rate_400 = 0b1100,
  TransportRx_SetOutputDataRate_Rate_200 = 0b1011,
  TransportRx_SetOutputDataRate_Rate_100 = 0b1010,
  TransportRx_SetOutputDataRate_Rate_50 = 0b1001
};

/**
 * Range flags as described in data sheet.
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum TransportRx_SetRange_Range {
  TransportRx_SetRange_Range_2g = 0b00,
  TransportRx_SetRange_Range_4g = 0b01,
  TransportRx_SetRange_Range_8g = 0b10,
  TransportRx_SetRange_Range_16g = 0b11
};

/**
 * Scale flags as described in data sheet.
 * See section Register Map in ADXL345 Data Sheet Rev.G (pp.24-28)
 */
enum TransportRx_SetScale_Scale {
  TransportRx_SetScale_Scale_10bit = 0,
  TransportRx_SetScale_Scale_full4mg
};

/**
 * RX payload for retrieving sensor's ODR.
 */
struct TransportRx_GetOutputDataRate {
} __attribute__((packed));

/**
 * RX payload for setting sensor's ODR.
 */
struct TransportRx_SetOutputDataRate {
  enum TransportRx_SetOutputDataRate_Rate rate;
} __attribute__((packed));

/**
 * RX payload for retrieving sensor's range.
 */
struct TransportRx_GetRange {
} __attribute__((packed));

/**
 * RX payload for setting sensor's range.
 */
struct TransportRx_SetRange {
  enum TransportRx_SetRange_Range range;
} __attribute__((packed));

/**
 * RX payload for retrieving sensor's scale.
 */
struct TransportRx_GetScale {
} __attribute__((packed));

/**
 * RX payload for setting sensor's scale.
 */
struct TransportRx_SetScale {
  enum TransportRx_SetScale_Scale scale;
} __attribute__((packed));

/**
 * RX payload for requesting controller reboot.
 */
struct TransportRx_DeviceReboot {
} __attribute__((packed));

/**
 * RX payload for requesting sampling start.
 */
struct TransportRx_SamplingStart {
  uint16_t max_samples_count;
} __attribute__((packed));

/**
 * RX payload for requesting sampling stop.
 */
struct TransportRx_SamplingStop {
} __attribute__((packed));

/**
 * RX payload for retrieving sensor's mot relevant settings.
 */
struct TransportRx_GetDeviceSetup {
} __attribute__((packed));

/**
 * RX payload for retrieving the firmware version.
 */
struct TransportRx_GetFirmwareVersion {
} __attribute__((packed));

/* TX ------------------------------------------------------------------------*/

/**
 * TX payload response with sensor's ODR.
 */
struct TransportTx_OutputDataRate {
  enum TransportRx_SetOutputDataRate_Rate rate;
} __attribute__((packed));

/**
 * TX payload response with sensor's range.
 */
struct TransportTx_Range {
  enum TransportRx_SetRange_Range range;
} __attribute__((packed));

/**
 * TX payload response with sensor's scale.
 */
struct TransportTx_Scale {
  enum TransportRx_SetScale_Scale scale;
} __attribute__((packed));

/**
 * TX payload indicating FiFo overrun occurred.
 */
struct TransportTx_FifoOverflow {
} __attribute__((packed));

/**
 * TX payload indicating sampling started.
 */
struct TransportTx_SamplingStarted {
  uint16_t maxSamples;
} __attribute__((packed));

/**
 * TX payload indicating sampling stream has finished (all samples sent).
 *
 * Response to host indicating that sampling has been finished successfully.
 * This package is sent at the end of stream.
 * When this response is sent, the sampling stream has been successfully
 * transferred:
 *  - stream is complete and
 *  - without HW errors.
 */
struct TransportTx_SamplingFinished {
} __attribute__((packed));

/**
 * TX payload transporting most relevant device settings.
 */
struct TransportTx_DeviceSetup {
  uint8_t outputDataRate : 4; ///< \see Adxl345Register_BwRate_Rate
  uint8_t range : 2;          ///< \see Adxl345Register_DataFormat_Range
  uint8_t scale : 1;          ///< \see Adxl345Register_DataFormat_FullResBit
} __attribute__((packed));

/**
 * TX payload indicating that the sampling has been stopped (for whatever
 * reason).
 *
 * This package is sent whenever the device stops gathering data from sensor.
 * Reasons: user request, overflow, error
 */
struct TransportTx_SamplingStopped {
} __attribute__((packed));

/**
 * TX payload indicating that the sampling has been aborted upon user request.
 *
 * Sent when user requested to stop forwarding samples was received.
 */
struct TransportTx_SamplingAborted {
} __attribute__((packed));

/**
 * TX payload transporting an acceleration sample.
 */
struct TransportTx_Acceleration {
  uint16_t index; ///< running sample index
  int16_t x;      ///< raw sampling value in x-axis as seen from sensor
  int16_t y;      ///< raw sampling value in y-axis as seen from sensor
  int16_t z;      ///< raw sampling value in z-axis as seen from sensor
} __attribute__((packed));

/**
 * TX payload transporting the firmware version.
 */
struct TransportTx_FirmwareVersion {
  uint8_t major; ///< major version
  uint8_t minor; ///< minor version
  uint8_t patch; ///< patch version
} __attribute__((packed));

/* Frames --------------------------------------------------------------------*/

/**
 * Union for convenient casting to respective TX type without c-style cast.
 */
union TransportTxFrame {
  struct TransportTx_OutputDataRate asOutputDataRate;
  struct TransportTx_Range asRange;
  struct TransportTx_Scale asScale;
  struct TransportTx_DeviceSetup asDeviceSetup;
  struct TransportTx_FifoOverflow asFifoOverflow;
  struct TransportTx_SamplingStarted asSamplingStarted;
  struct TransportTx_SamplingFinished asSamplingFinished;
  struct TransportTx_SamplingStopped asSamplingStopped;
  struct TransportTx_SamplingAborted asSamplingAborted;
  struct TransportTx_Acceleration asAcceleration;
  struct TransportTx_FirmwareVersion asFirmwareVersion;
} __attribute__((packed));

/**
 * Union for convenient casting to respective RX type without c-style cast.
 */
union TransportRxFrame {
  struct TransportRx_SetOutputDataRate asSetOutputDataRate;
  struct TransportRx_GetOutputDataRate asGetOutputDataRate;
  struct TransportRx_SetRange asSetRange;
  struct TransportRx_GetRange asGetRange;
  struct TransportRx_SetScale asSetScale;
  struct TransportRx_GetScale asGetScale;
  struct TransportRx_GetDeviceSetup asGetDeviceSetup;
  struct TransportRx_DeviceReboot asDeviceReboot;
  struct TransportRx_SamplingStart asSamplingStart;
  struct TransportRx_SamplingStop asSamplingStop;
  struct TransportRx_GetFirmwareVersion asGetFirmwareVersion;
} __attribute__((packed));

/**
 * Generic package (TX/RX) including header and payload.
 */
struct TransportFrame {
  struct Transport_Header header;
  union {
    union TransportTxFrame asTxFrame;
    union TransportRxFrame asRxFrame;
  };
} __attribute__((packed));
