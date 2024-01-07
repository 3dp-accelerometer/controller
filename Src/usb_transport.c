#include "usb_transport.h"
#include "adxl345.h"
#include "device_reboot.h"
#include "sampling.h"
#include "usbd_cdc_if.h"
#include "version.h"
#include <errno.h>

int TransportRxProcess(uint8_t *buffer, const uint32_t *length) {
  if (NULL == buffer || NULL == length)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
    // get firmware version
  case TransportHeader_Id_Rx_GetFirmwareVersion: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetFirmwareVersion) ==
        *length) {

      struct TransportFrame response = {
          .header.id = TransportHeader_Id_Tx_FirmwareVersion,
          .asTxFrame.asFirmwareVersion.major = VERSION_MAJOR,
          .asTxFrame.asFirmwareVersion.minor = VERSION_MINOR,
          .asTxFrame.asFirmwareVersion.patch = VERSION_PATCH};

      // send version
      while (USBD_BUSY ==
             CDC_Transmit_FS((uint8_t *)&response,
                             SIZEOF_HEADER_INCL_PAYLOAD(
                                 response.asTxFrame.asFirmwareVersion)))
        ;
      return 0;
    }
  } break;
    // get ODR
  case TransportHeader_Id_Rx_GetOutputDataRate: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetOutputDataRate) ==
        *length) {
      enum Adxl345Register_BwRate_Rate rate;
      Adxl345_getOutputDataRate(&rate);

      struct TransportFrame response = {
          .header.id = TransportHeader_Id_Tx_OutputDataRate};
      response.asTxFrame.asOutputDataRate.rate =
          (enum TransportRx_SetOutputDataRate_Rate)rate;
      // send ODR
      while (USBD_BUSY ==
             CDC_Transmit_FS((uint8_t *)&response,
                             SIZEOF_HEADER_INCL_PAYLOAD(
                                 response.asTxFrame.asOutputDataRate)))
        ;
      return 0;
    }
  } break;
    // set ODR
  case TransportHeader_Id_Rx_SetOutputDataRate:
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetOutputDataRate) ==
        *length) {
      return Adxl345_setOutputDataRate(
          request->asRxFrame.asSetOutputDataRate.rate);
    }
    break;

    // get range
  case TransportHeader_Id_Rx_GetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetRange) == *length) {
      enum Adxl345Register_DataFormat_Range range;
      Adxl345_getRange(&range);

      struct TransportFrame response = {.header.id =
                                            TransportHeader_Id_Tx_Range};
      response.asTxFrame.asRange.range = (enum TransportRx_SetRange_Range)range;

      // send range
      while (USBD_BUSY == CDC_Transmit_FS((uint8_t *)&response,
                                          SIZEOF_HEADER_INCL_PAYLOAD(
                                              response.asTxFrame.asRange)))
        ;
      return 0;
    }
  } break;

    // set range
  case TransportHeader_Id_Rx_SetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetRange) == *length) {
      return Adxl345_setRange(request->asRxFrame.asSetRange.range);
    }
  } break;

    // get scale
  case TransportHeader_Id_Rx_GetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetScale) == *length) {
      enum Adxl345Register_DataFormat_FullResBit scale;
      Adxl345_getScale(&scale);

      struct TransportFrame response = {.header.id =
                                            TransportHeader_Id_Tx_Scale};
      response.asTxFrame.asScale.scale = (enum TransportRx_SetScale_Scale)scale;

      // send scale
      while (USBD_BUSY == CDC_Transmit_FS((uint8_t *)&response,
                                          SIZEOF_HEADER_INCL_PAYLOAD(
                                              response.asTxFrame.asScale)))
        ;
      return 0;
    }
  } break;

    // set scale
  case TransportHeader_Id_Rx_SetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetScale) == *length) {
      return Adxl345_setScale(request->asRxFrame.asSetScale.scale);
    }
  } break;

  // get device setup
  case TransportHeader_Id_Rx_GetDeviceSetup: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetDeviceSetup) ==
        *length) {
      enum Adxl345Register_BwRate_Rate rate;
      enum Adxl345Register_DataFormat_Range range;
      enum Adxl345Register_DataFormat_FullResBit scale;
      Adxl345_getOutputDataRate(&rate);
      Adxl345_getRange(&range);
      Adxl345_getScale(&scale);

      struct TransportFrame response = {.header.id =
                                            TransportHeader_Id_Tx_DeviceSetup};
      response.asTxFrame.asDeviceSetup.outputDataRate =
          (enum TransportRx_SetOutputDataRate_Rate)rate;
      response.asTxFrame.asDeviceSetup.range =
          (enum TransportRx_SetRange_Range)range;
      response.asTxFrame.asDeviceSetup.scale =
          (enum TransportRx_SetScale_Scale)scale;

      // send scale
      while (USBD_BUSY ==
             CDC_Transmit_FS(
                 (uint8_t *)&response,
                 SIZEOF_HEADER_INCL_PAYLOAD(response.asTxFrame.asDeviceSetup)))
        ;
      return 0;
    }
  } break;

    // device reboot requested
  case TransportHeader_Id_Rx_DeviceReboot: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_DeviceReboot) ==
        *length) {
      device_reboot_requestAsyncReboot();
      return 0;
    }
  } break;

    // sapling start requested
  case TransportHeader_Id_Rx_SamplingStart: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStart) ==
        *length) {
      sampling_start(request->asRxFrame.asSamplingStart.max_samples_count);
      return 0;
    }
  } break;

    // sampling stop requested
  case TransportHeader_Id_Rx_SamplingStop: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStop) ==
        *length) {
      sampling_stop();
      return 0;
    }
  } break;

  default:
    break;
  }

  return -EINVAL;
}

void TransportTxSamplingSetup() {
  struct TransportFrame tx;

  enum Adxl345Register_BwRate_Rate rate;
  enum Adxl345Register_DataFormat_Range range;
  enum Adxl345Register_DataFormat_FullResBit scale;

  Adxl345_getOutputDataRate(&rate);
  Adxl345_getScale(&scale);
  Adxl345_getRange(&range);

  tx.header.id = TransportHeader_Id_Tx_DeviceSetup;
  tx.asTxFrame.asDeviceSetup.outputDataRate = rate;
  tx.asTxFrame.asDeviceSetup.range = range;
  tx.asTxFrame.asDeviceSetup.scale = scale;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx, SIZEOF_HEADER_INCL_PAYLOAD(
                                             tx.asTxFrame.asDeviceSetup)))
    ;
}
void TransportTxFirmwareVersion() {
  struct TransportFrame tx;
  tx.header.id = TransportHeader_Id_Tx_FirmwareVersion;
  tx.asTxFrame.asFirmwareVersion.major = VERSION_MAJOR;
  tx.asTxFrame.asFirmwareVersion.minor = VERSION_MINOR;
  tx.asTxFrame.asFirmwareVersion.patch = VERSION_PATCH;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx, SIZEOF_HEADER_INCL_PAYLOAD(
                                             tx.asTxFrame.asFirmwareVersion)))
    ;
}

void TransportTxSamplingStarted(uint16_t max_samples) {
  struct TransportFrame tx;
  tx.header.id = TransportHeader_Id_Tx_SamplingStarted;
  tx.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct TransportHeader) +
                             sizeof(tx.asTxFrame.asSamplingStarted)))
    ;
}

void TransportTxSamplingFinished() {
  struct TransportFrame tx = {.header.id =
                                  TransportHeader_Id_Tx_SamplingFinished};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct TransportHeader) +
                             sizeof(tx.asTxFrame.asSamplingFinished)))
    ;
}

void TransportTxSamplingStopped() {
  TransportTxSamplingSetup();

  struct TransportFrame tx = {.header.id =
                                  TransportHeader_Id_Tx_SamplingStopped};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct TransportHeader) +
                             sizeof(tx.asTxFrame.asSamplingStopped)))
    ;
}

void TransportTxSamplingAborted() {
  struct TransportFrame tx = {.header.id =
                                  TransportHeader_Id_Tx_SamplingAborted};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct TransportHeader) +
                             sizeof(tx.asTxFrame.asSamplingAborted)))
    ;
}

void TransportTxFifoOverflow() {
  struct TransportFrame tx = {.header.id = TransportHeader_Id_Tx_FifoOverflow};
  while (USBD_BUSY == CDC_Transmit_FS((uint8_t *)&tx,
                                      sizeof(struct TransportHeader) +
                                          sizeof(tx.asTxFrame.asFifoOverflow)))
    ;
}

int TransportTxAccelerationBuffer(struct Adxl345_Acceleration *buffer,
                                  uint8_t count, uint16_t start_index) {
  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  struct TransportFrame acc[ADXL345_WATERMARK_LEVEL] = {};

  if (ADXL345_WATERMARK_LEVEL < count || NULL == buffer) {
    return -EINVAL;
  }

  // todo
  assert(sizeof(struct TransportFrame) ==
         (sizeof(struct TransportTx_Acceleration) +
          sizeof(struct TransportHeader)));

  for (uint8_t idx = 0; idx < count; idx++) {
    acc[idx].asTxFrame.asAcceleration.index = start_index++;
    acc[idx].asTxFrame.asAcceleration.x = buffer[idx].x;
    acc[idx].asTxFrame.asAcceleration.y = buffer[idx].y;
    acc[idx].asTxFrame.asAcceleration.z = buffer[idx].z;
    acc[idx].header.id = TransportHeader_Id_Tx_Acceleration;
  }

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)acc, count * sizeof(struct TransportFrame)))
    ;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
  return 0;
}
