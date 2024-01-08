#include "fw/usbd_cdc_transport.h"
#include "fw/adxl345.h"
#include "fw/adxl345_transport_types.h"
#include "fw/device_reboot.h"
#include "fw/sampling.h"
#include "fw/usbd_cdc_transport_types.h"
#include "fw/version.h"
#include "usbd_cdc_if.h"
#include <errno.h>

int TransportRx_Process(uint8_t *buffer, const uint32_t *length) {
  if (NULL == buffer || NULL == length)
    return -EINVAL;

  struct TransportFrame *request = (struct TransportFrame *)buffer;

  switch (request->header.id) {
    // get firmware version
  case Transport_HeaderId_Rx_GetFirmwareVersion: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetFirmwareVersion) ==
        *length) {

      struct TransportFrame response = {
          .header.id = Transport_HeaderId_Tx_FirmwareVersion,
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
  case Transport_HeaderId_Rx_GetOutputDataRate: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetOutputDataRate) ==
        *length) {
      enum Adxl345Register_BwRate_Rate rate;
      Adxl345_getOutputDataRate(&rate);

      struct TransportFrame response = {
          .header.id = Transport_HeaderId_Tx_OutputDataRate};
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
  case Transport_HeaderId_Rx_SetOutputDataRate:
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetOutputDataRate) ==
        *length) {
      return Adxl345_setOutputDataRate(
          request->asRxFrame.asSetOutputDataRate.rate);
    }
    break;

    // get range
  case Transport_HeaderId_Rx_GetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetRange) == *length) {
      enum Adxl345Register_DataFormat_Range range;
      Adxl345_getRange(&range);

      struct TransportFrame response = {.header.id =
                                            Transport_HeaderId_Tx_Range};
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
  case Transport_HeaderId_Rx_SetRange: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetRange) == *length) {
      return Adxl345_setRange(request->asRxFrame.asSetRange.range);
    }
  } break;

    // get scale
  case Transport_HeaderId_Rx_GetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetScale) == *length) {
      enum Adxl345Register_DataFormat_FullResBit scale;
      Adxl345_getScale(&scale);

      struct TransportFrame response = {.header.id =
                                            Transport_HeaderId_Tx_Scale};
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
  case Transport_HeaderId_Rx_SetScale: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SetScale) == *length) {
      return Adxl345_setScale(request->asRxFrame.asSetScale.scale);
    }
  } break;

  // get device setup
  case Transport_HeaderId_Rx_GetDeviceSetup: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_GetDeviceSetup) ==
        *length) {
      enum Adxl345Register_BwRate_Rate rate;
      enum Adxl345Register_DataFormat_Range range;
      enum Adxl345Register_DataFormat_FullResBit scale;
      Adxl345_getOutputDataRate(&rate);
      Adxl345_getRange(&range);
      Adxl345_getScale(&scale);

      struct TransportFrame response = {.header.id =
                                            Transport_HeaderId_Tx_DeviceSetup};
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
  case Transport_HeaderId_Rx_DeviceReboot: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_DeviceReboot) ==
        *length) {
      DeviceReboot_requestAsyncReboot();
      return 0;
    }
  } break;

    // sapling start requested
  case Transport_HeaderId_Rx_SamplingStart: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStart) ==
        *length) {
      Sampling_start(request->asRxFrame.asSamplingStart.max_samples_count);
      return 0;
    }
  } break;

    // sampling stop requested
  case Transport_HeaderId_Rx_SamplingStop: {
    if (SIZEOF_HEADER_INCL_PAYLOAD(struct TransportRx_SamplingStop) ==
        *length) {
      Sampling_stop();
      return 0;
    }
  } break;

  default:
    break;
  }

  return -EINVAL;
}

void TransportTx_SamplingSetup() {
  struct TransportFrame tx;

  enum Adxl345Register_BwRate_Rate rate;
  enum Adxl345Register_DataFormat_Range range;
  enum Adxl345Register_DataFormat_FullResBit scale;

  Adxl345_getOutputDataRate(&rate);
  Adxl345_getScale(&scale);
  Adxl345_getRange(&range);

  tx.header.id = Transport_HeaderId_Tx_DeviceSetup;
  tx.asTxFrame.asDeviceSetup.outputDataRate = rate;
  tx.asTxFrame.asDeviceSetup.range = range;
  tx.asTxFrame.asDeviceSetup.scale = scale;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx, SIZEOF_HEADER_INCL_PAYLOAD(
                                             tx.asTxFrame.asDeviceSetup)))
    ;
}

void TransportTx_FirmwareVersion() {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_FirmwareVersion;
  tx.asTxFrame.asFirmwareVersion.major = VERSION_MAJOR;
  tx.asTxFrame.asFirmwareVersion.minor = VERSION_MINOR;
  tx.asTxFrame.asFirmwareVersion.patch = VERSION_PATCH;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx, SIZEOF_HEADER_INCL_PAYLOAD(
                                             tx.asTxFrame.asFirmwareVersion)))
    ;
}

void TransportTx_SamplingStarted(uint16_t max_samples) {
  struct TransportFrame tx;
  tx.header.id = Transport_HeaderId_Tx_SamplingStarted;
  tx.asTxFrame.asSamplingStarted.maxSamples = max_samples;

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct Transport_Header) +
                             sizeof(tx.asTxFrame.asSamplingStarted)))
    ;
}

void TransportTx_SamplingFinished() {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingFinished};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct Transport_Header) +
                             sizeof(tx.asTxFrame.asSamplingFinished)))
    ;
}

void TransportTx_SamplingStopped() {
  TransportTx_SamplingSetup();

  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingStopped};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct Transport_Header) +
                             sizeof(tx.asTxFrame.asSamplingStopped)))
    ;
}

void TransportTx_SamplingAborted() {
  struct TransportFrame tx = {.header.id =
                                  Transport_HeaderId_Tx_SamplingAborted};
  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)&tx,
                         sizeof(struct Transport_Header) +
                             sizeof(tx.asTxFrame.asSamplingAborted)))
    ;
}

void TransportTx_FifoOverflow() {
  struct TransportFrame tx = {.header.id = Transport_HeaderId_Tx_FifoOverflow};
  while (USBD_BUSY == CDC_Transmit_FS((uint8_t *)&tx,
                                      sizeof(struct Transport_Header) +
                                          sizeof(tx.asTxFrame.asFifoOverflow)))
    ;
}

int TransportTx_AccelerationBuffer(struct Adxl345TP_Acceleration *data,
                                   uint8_t count, uint16_t start_index) {
  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET);
  struct TransportFrame acc[ADXL345_WATERMARK_LEVEL] = {};

  if (ADXL345_WATERMARK_LEVEL < count || NULL == data) {
    return -EINVAL;
  }

  // todo
  assert(sizeof(struct TransportFrame) ==
         (sizeof(struct TransportTx_Acceleration) +
          sizeof(struct Transport_Header)));

  for (uint8_t idx = 0; idx < count; idx++) {
    acc[idx].asTxFrame.asAcceleration.index = start_index++;
    acc[idx].asTxFrame.asAcceleration.x = data[idx].x;
    acc[idx].asTxFrame.asAcceleration.y = data[idx].y;
    acc[idx].asTxFrame.asAcceleration.z = data[idx].z;
    acc[idx].header.id = Transport_HeaderId_Tx_Acceleration;
  }

  // todo: don't insist for completed transmission here. usb host (pc) will poll
  //   the usb client (us) for transactions about every 1ms or lesser. this is
  //   even more visible on weak hardware (i.e. raspberry pi).
  //   suggestion:
  //     1 apply Ringbuffer from <ringbuffer.h>
  //     2 store complete Adxl345TP_Acceleration buffer to ringbuffer
  //     3 try to send from ringbuffer: several items, as many possible but timeboxed
  //     4 return ENODATA (nothing to transmit) or EBUSY (data pending) respectively
  //     5 send remaining next time we are called
  //     6 caller shall call us until ENODATA

  while (USBD_BUSY ==
         CDC_Transmit_FS((uint8_t *)acc, count * sizeof(struct TransportFrame)))
    ;

  // HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET);
  return 0;
}
