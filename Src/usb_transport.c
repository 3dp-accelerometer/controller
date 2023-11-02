#include "usb_transport.h"
#include "adxl345.h"
#include "usbd_cdc_if.h"
#include <errno.h>

#define EXPECTED_RX_PKG_SIZE(packageType)                                      \
  (sizeof(packageType) + sizeof(struct TransportHeader))

int TransportRxProcess(uint8_t *buffer, uint32_t *length) {
  if (NULL == buffer || NULL == length)
    return -EINVAL;

  struct TransportFrame *frame = (struct TransportFrame *)buffer;

  switch (frame->header.id) {
  case TransportHeader_Id_GetOutputDataRate: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_GetOutputDataRate) == *length) {
      enum Adxl345Register_BwRate_Rate rate;
      Adxl345_getOutputDataRate(&rate);
      union TransportTxFrame txFrame = {
          .asGetOutputDataRate.rate =
              (enum TransportRx_SetOutputDataRate_Rate)rate};
      CDC_Transmit_FS((uint8_t *)&txFrame, sizeof(txFrame.asGetOutputDataRate));
    }
  } break;

  case TransportHeader_Id_SetOutputDataRate:
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SetOutputDataRate) == *length) {
      return Adxl345_setOutputDataRate(
          frame->asRxFrame.asSetOutputDataRate.rate);
    }
    break;

  case TransportHeader_Id_GetRange: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_GetRange) == *length) {
      enum Adxl345Register_DataFormat_Range range;
      Adxl345_getRange(&range);
      union TransportTxFrame txFrame = {
          .asGetRange.range = (enum TransportRx_SetRange_Range)range};
      CDC_Transmit_FS((uint8_t *)&txFrame, sizeof(txFrame.asGetRange));
    }
  } break;

  case TransportHeader_Id_SetRange: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SetRange) == *length) {
      return Adxl345_setRange(frame->asRxFrame.asSetRange.range);
    }
  } break;

  case TransportHeader_Id_GetScale: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_GetScale) == *length) {
      enum Adxl345Register_DataFormat_FullResBit scale;
      Adxl345_getScale(&scale);
      union TransportTxFrame txFrame = {
          .asGetScale.scale = (enum TransportRx_SetScale_Scale)scale};
      CDC_Transmit_FS((uint8_t *)&txFrame, sizeof(txFrame.asGetScale));
    }
  } break;

  case TransportHeader_Id_SetScale: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SetScale) == *length) {
      return Adxl345_setScale(frame->asRxFrame.asSetScale.scale);
    }
  } break;

  default:
    break;
  }

  char str[24];
  sprintf(str, "unknown command=%d\r\n", (uint8_t)frame->header.id);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return -EINVAL;
}
