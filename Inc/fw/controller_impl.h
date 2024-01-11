/**
 * \file device_impl.h
 *
 * API of device implementation.
 */

#pragma once

#include <adxl345.h>
#include <controller.h>
#include <host_transport.h>
#include <inttypes.h>
#include <sampling.h>

struct Sampling_Acceleration;

enum TransportRx_SetOutputDataRate_Rate;
enum TransportRx_SetRange_Range;
enum TransportRx_SetScale_Scale;

// extern struct Adxl345_Handle sensorHandle;
// extern struct Samplint_Handle samplingHandle;
// extern struct HostTransport_Handle hostTransportHandle;
// extern struct Controller_Handle controllerHandle;

#define DEVICEIMPL_DECLARE_HANDLE(HANDLE_NAME, SENSOR_HANDLE_NAME,             \
                                  SAMPLING_HANDLE_NAME,                        \
                                  HOST_TRANSPORT_HANDLE_NAME)                  \
  struct Controller_Handle HANDLE_NAME = {                                     \
      .swVersionMajor = VERSION_MAJOR,                                         \
      .swVersionMinor = VERSION_MINOR,                                         \
      .swVersionPatch = VERSION_PATCH,                                         \
                                                                               \
      .sensorHandle = &SENSOR_HANDLE_NAME,                                     \
      .samplingHandle = &SAMPLING_HANDLE_NAME,                                 \
      .hostTransportHandle = &HOST_TRANSPORT_HANDLE_NAME,                      \
                                                                               \
      .init = ControllerImpl_init,                                             \
      .loop = ControllerImpl_loop,                                             \
                                                                               \
      .sensorInit = ControllerImpl_sensor_Adxl345_init,                        \
                                                                               \
      .controllerCheckReboot = ControllerImpl_device_checkReboot,              \
      .controllerRequestReboot = ControllerImpl_device_requestAsyncReboot,     \
                                                                               \
      .hostOnBytesReceived = ControllerImpl_host_onBytesReceived,              \
      .hostOnRequestGetFirmwareVersion =                                       \
          ControllerImpl_host_onRequestGetFirmwareVersion,                     \
      .hostOnRequestGetOutputDataRate =                                        \
          ControllerImpl_host_onRequestGetOutputDataRate,                      \
      .hostOnRequestSetOutputDatatRate =                                       \
          ControllerImpl_host_onRequestSetOutputDatatRate,                     \
      .hostOnRequestGetRange = ControllerImpl_host_onRequestGetRange,          \
      .hostOnRequestSetRange = ControllerImpl_host_onRequestSetRange,          \
      .hostOnRequestGetScale = ControllerImpl_host_onRequestGetScale,          \
      .hostOnRequestSetScale = ControllerImpl_host_onRequestSetScale,          \
      .hostOnRequestGetDeviceSetup =                                           \
          ControllerImpl_host_onRequestGetDeviceSetup,                         \
      .hostOnRequestSamplingStart =                                            \
          ControllerImpl_host_onRequestSamplingStart,                          \
      .hostOnRequestSamplingStop = ControllerImpl_host_onRequestSamplingStop,  \
                                                                               \
      .samplingStart = ControllerImpl_sampling_start,                          \
      .samplingStop = ControllerImpl_sampling_stop,                            \
      .samplingFetchForward = ControllerImpl_sampling_fetchForward,            \
      .samplingSetFifoWatermark = ControllerImpl_sampling_setFifoWatermark,    \
      .samplingClearFifoWatermark =                                            \
          ControllerImpl_sampling_clearFifoWatermark,                          \
      .samplingSetFifoOverflow = ControllerImpl_sampling_setFifoOverflow,      \
      .samplingOn5usTimerExpired = ControllerImpl_sampling_on5usTimerExpired,  \
      .samplingOnStarted = ControllerImpl_sampling_onSamplingStarted,          \
      .samplingOnStopped = ControllerImpl_sampling_onSamplingStopped,          \
      .samplingOnAborted = ControllerImpl_sampling_onSamplingAborted,          \
      .samplingOnFinished = ControllerImpl_sampling_onSamplingFinished,        \
      .samplingOnPostAccelerationBuffer =                                      \
          ControllerImpl_sampling_onPostAccelerationBuffer,                    \
      .samplingOnFifoOverflow = ControllerImpl_sampling_onFifoOverflow,        \
      .samplingOnSensorEnable = ControllerImpl_sampling_onSensorEnable,        \
      .samplingOnSensorDisable = ControllerImpl_sampling_onSensorDisable,      \
      .samplingOnFetchSensorAcceleration =                                     \
          ControllerImpl_sampling_onFetchSensorAcceleration,                   \
  }

void ControllerImpl_init();
void ControllerImpl_loop();

void ControllerImpl_device_checkReboot();
void ControllerImpl_device_requestAsyncReboot();

/**
 * Handles incoming bytes (unfragmented data packet).
 *
 * Calls generic TransportRx_Process(uint8_t *buffer, uint16_t length) implementation which performs basic
 * checks only. Further processing/dispatching is delegated to the respective
 * pimpl. \see host_transport_impl.h
 *
 * @param buffer received byte buffer (must not be fragmented)
 * @param len received data length
 */
void ControllerImpl_host_onBytesReceived(uint8_t *buffer, uint16_t len);
int ControllerImpl_host_onRequestGetFirmwareVersion();
int ControllerImpl_host_onRequestGetOutputDataRate();
int ControllerImpl_host_onRequestSetOutputDatatRate(
    enum TransportRx_SetOutputDataRate_Rate odr);
int ControllerImpl_host_onRequestGetRange();
int ControllerImpl_host_onRequestSetRange(
    enum TransportRx_SetRange_Range range);
int ControllerImpl_host_onRequestGetScale();
int ControllerImpl_host_onRequestSetScale(
    enum TransportRx_SetScale_Scale scale);
int ControllerImpl_host_onRequestGetDeviceSetup();
int ControllerImpl_host_onRequestSamplingStart(uint16_t maxSamplesCount);
int ControllerImpl_host_onRequestSamplingStop();

void ControllerImpl_sensor_Adxl345_init();

void ControllerImpl_sampling_start(uint16_t maxSamples);
void ControllerImpl_sampling_stop();
int ControllerImpl_sampling_fetchForward();
void ControllerImpl_sampling_setFifoWatermark();
void ControllerImpl_sampling_clearFifoWatermark();
void ControllerImpl_sampling_setFifoOverflow();
void ControllerImpl_sampling_on5usTimerExpired();
void ControllerImpl_sampling_onSamplingStarted();
void ControllerImpl_sampling_onSamplingStopped();
void ControllerImpl_sampling_onSamplingAborted();
void ControllerImpl_sampling_onSamplingFinished();
void ControllerImpl_sampling_onPostAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex);
void ControllerImpl_sampling_onFifoOverflow();
void ControllerImpl_sampling_onSensorEnable();
void ControllerImpl_sampling_onSensorDisable();
void ControllerImpl_sampling_onFetchSensorAcceleration(
    struct Sampling_Acceleration *sample);
