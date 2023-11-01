#pragma once
#include <inttypes.h>

/* Transport Header ----------------------------------------------------------*/

enum TransportHeader_Id {
  TransportHeader_Id_SetOutputDataRate,
  TransportHeader_Id_SetRange
};

struct TransportHeader {
  enum TransportHeader_Id id;
} __attribute__((packed));

/* TX ------------------------------------------------------------------------*/

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

struct TransportRx_SetOutputDataRate {
  enum TransportRx_SetOutputDataRate_Rate rate;
};

struct TransportRx_SetRange {
  enum TransportRx_SetRange_Range range;
};

struct TransportTxResponseBar {};

/* RX ------------------------------------------------------------------------*/

union TransportTxFrame {
  struct TransportTxResponseBar asResponseBar;
} __attribute__((packed));

union TransportRxFrame {
  struct TransportRx_SetOutputDataRate asSetOutputDataRate;
  struct TransportRx_SetRange asSetRange;

} __attribute__((packed));

struct TransportFrame {
  struct TransportHeader header;
  union {
    union TransportTxFrame asTxFrame;
    union TransportRxFrame asRxFrame;
  };
} __attribute__((packed));

/* ---------------------------------------------------------------------------*/

int TransportRxProcess(uint8_t *buffer, uint32_t *length);
