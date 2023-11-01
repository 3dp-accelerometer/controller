#include "usb_transport.h"
#include "adxl345.h"
#include "usbd_cdc_if.h"
#include <errno.h>

int TransportRxProcess(uint8_t *buffer, uint32_t *length) {
  if (NULL == buffer || NULL == length)
    return -EINVAL;

  struct TransportFrame *frame = (struct TransportFrame *)buffer;

  switch (frame->header.id) {
  case TransportHeader_Id_SetOutputDataRate:
    if (sizeof(struct TransportRx_SetOutputDataRate) == *length)
      return Adxl345_setOutputDataRate(
          frame->asRxFrame.asSetOutputDataRate.rate);
    else
      break;

  case TransportHeader_Id_SetRange:
    if (sizeof(struct TransportRx_SetRange) == *length)
      return Adxl345_setRange(frame->asRxFrame.asSetRange.range);
    else
      break;

  default:
    break;
  }

  char str[24];
  sprintf(str, "unknown command=%d\r\n", (uint8_t)frame->header.id);
  CDC_Transmit_FS((uint8_t *)str, strnlen(str, sizeof(str)));

  return -EINVAL;
}
