#include "usb_transport.h"
#include "adxl345.h"
#include "device_reboot.h"
#include "sampling.h"
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
      return CDC_Transmit_FS((uint8_t *)&txFrame,
                             sizeof(txFrame.asGetOutputDataRate));
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
      return CDC_Transmit_FS((uint8_t *)&txFrame, sizeof(txFrame.asGetRange));
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
      return CDC_Transmit_FS((uint8_t *)&txFrame, sizeof(txFrame.asGetScale));
    }
  } break;

  case TransportHeader_Id_SetScale: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SetScale) == *length) {
      return Adxl345_setScale(frame->asRxFrame.asSetScale.scale);
    }
  } break;

  case TransportHeader_Id_DeviceReboot: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_DeviceReboot) == *length) {
      device_reboot_requestAsyncReboot();
      return 0;
    }
  } break;

  case TransportHeader_Id_SamplingStart: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SamplingStart) == *length) {
      return sampling_start();
    }
  } break;

  case TransportHeader_Id_SamplingStartN: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SamplingStartN) == *length) {
      return sampling_startN(
          frame->asRxFrame.asSamplingStartN.max_samples_count);
    }
  }

  break;
  case TransportHeader_Id_SamplingStop: {
    if (EXPECTED_RX_PKG_SIZE(struct TransportRx_SamplingStop) == *length) {
      return sampling_stop();
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
