#pragma once
#include <inttypes.h>

/*----------------------------------------------------------------------------*/

struct Adxl345_Acceleration;

/*----------------------------------------------------------------------------*/

#define SIZEOF_HEADER_INCL_PAYLOAD(packageType)                                \
  (sizeof(struct TransportHeader) + sizeof(packageType))

/* Transport Header ----------------------------------------------------------*/

enum TransportHeader_Id {
  TransportHeader_Id_Rx_SetOutputDataRate = 1,
  TransportHeader_Id_Rx_GetOutputDataRate,
  TransportHeader_Id_Rx_SetRange,
  TransportHeader_Id_Rx_GetRange,
  TransportHeader_Id_Rx_SetScale,
  TransportHeader_Id_Rx_GetScale,
  TransportHeader_Id_Rx_GetDeviceSetup,

  TransportHeader_Id_Rx_DeviceReboot = 17,
  TransportHeader_Id_Rx_SamplingStart,
  TransportHeader_Id_Rx_SamplingStop,

  TransportHeader_Id_Tx_OutputDataRate = 25,
  TransportHeader_Id_Tx_Range,
  TransportHeader_Id_Tx_Scale,
  TransportHeader_Id_Tx_DeviceSetup,

  TransportHeader_Id_Tx_FifoOverflow = 33,
  TransportHeader_Id_Tx_SamplingStarted,
  TransportHeader_Id_Tx_SamplingFinished,
  TransportHeader_Id_Tx_SamplingStopped,
  TransportHeader_Id_Tx_SamplingAborted,
  TransportHeader_Id_Tx_Acceleration,
};

struct TransportHeader {
  enum TransportHeader_Id id;
} __attribute__((packed));

/* RX ------------------------------------------------------------------------*/

enum TransportRx_SetOutputDataRate_Rate {
  TransportRx_SetOutputDataRate_Rate3200 = 0b1111,
  TransportRx_SetOutputDataRate_Rate1600 = 0b1110,
  TransportRx_SetOutputDataRate_Rate_800 = 0b1101,
  TransportRx_SetOutputDataRate_Rate_400 = 0b1100,
  TransportRx_SetOutputDataRate_Rate_200 = 0b1011,
  TransportRx_SetOutputDataRate_Rate_100 = 0b1010,
  TransportRx_SetOutputDataRate_Rate_50 = 0b1001
};

enum TransportRx_SetRange_Range {
  TransportRx_SetRange_Range_2g = 0b00,
  TransportRx_SetRange_Range_4g = 0b01,
  TransportRx_SetRange_Range_8g = 0b10,
  TransportRx_SetRange_Range_16g = 0b11
};

enum TransportRx_SetScale_Scale {
  TransportRx_SetScale_Scale_10bit = 0,
  TransportRx_SetScale_Scale_full4mg
};

struct TransportRx_GetOutputDataRate {
} __attribute__((packed));

struct TransportRx_SetOutputDataRate {
  enum TransportRx_SetOutputDataRate_Rate rate;
} __attribute__((packed));

struct TransportRx_GetRange {
} __attribute__((packed));

struct TransportRx_SetRange {
  enum TransportRx_SetRange_Range range;
} __attribute__((packed));

struct TransportRx_GetScale {
} __attribute__((packed));

struct TransportRx_SetScale {
  enum TransportRx_SetScale_Scale scale;
} __attribute__((packed));

struct TransportRx_DeviceReboot {
} __attribute__((packed));

struct TransportRx_SamplingStart {
  uint16_t max_samples_count;
} __attribute__((packed));

struct TransportRx_SamplingStop {
} __attribute__((packed));

struct TransportRx_GetDeviceSetup {
} __attribute__((packed));

/* TX ------------------------------------------------------------------------*/

struct TransportTx_OutputDataRate {
  enum TransportRx_SetOutputDataRate_Rate rate;
} __attribute__((packed));

struct TransportTx_Range {
  enum TransportRx_SetRange_Range range;
} __attribute__((packed));

struct TransportTx_Scale {
  enum TransportRx_SetScale_Scale scale;
} __attribute__((packed));

struct TransportTx_FifoOverflow {
} __attribute__((packed));

struct TransportTx_SamplingStarted {
  uint16_t maxSamples;
} __attribute__((packed));

struct TransportTx_SamplingFinished {
} __attribute__((packed));

struct TransportTx_DeviceSetup {
  uint8_t outputDataRate : 4; // enum Adxl345Register_BwRate_Rate
  uint8_t range : 2;          // enum Adxl345Register_DataFormat_Range
  uint8_t scale : 1;          // enum Adxl345Register_DataFormat_FullResBit
} __attribute__((packed));

struct TransportTx_SamplingStopped {
} __attribute__((packed));

struct TransportTx_SamplingAborted {
} __attribute__((packed));

struct TransportTx_Acceleration {
  uint16_t index;
  int16_t x;
  int16_t y;
  int16_t z;
} __attribute__((packed));

/* Frames --------------------------------------------------------------------*/

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
} __attribute__((packed));

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
} __attribute__((packed));

struct TransportFrame {
  struct TransportHeader header;
  union {
    union TransportTxFrame asTxFrame;
    union TransportRxFrame asRxFrame;
  };
} __attribute__((packed));

/* ---------------------------------------------------------------------------*/

int TransportRxProcess(uint8_t *buffer, const uint32_t *length);

/* ---------------------------------------------------------------------------*/

void TransportTxSamplingSetup();
void TransportTxSamplingStarted(uint16_t max_samples);
void TransportTxSamplingFinished();
void TransportTxSamplingStopped();
void TransportTxSamplingAborted();
void TransportTxFifoOverflow();
int TransportTxAccelerationBuffer(struct Adxl345_Acceleration *buffer,
                                  uint8_t count, uint16_t start_index);
