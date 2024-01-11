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

void ControllerImpl_init();
void ControllerImpl_loop();

void ControllerImpl_device_checkReboot();
void ControllerImpl_device_requestAsyncReboot();

/**
 * Handles incoming bytes (unfragmented data packet).
 *
 * Calls generic TransportRx_Process(uint8_t *buffer, uint16_t length)
 * implementation which performs basic checks only. Further
 * processing/dispatching is delegated to the respective pimpl. \see
 * host_transport_impl.h
 *
 * @param buffer received byte buffer (must not be fragmented)
 * @param len received data length
 */
void ControllerImpl_host_doTakeBytes(uint8_t *buffer, uint16_t len);
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
int ControllerImpl_sensor_Adxl345_getOutputDataRate(uint8_t *odr);
int ControllerImpl_sensor_Adxl345_getScale(uint8_t *scale);
int ControllerImpl_sensor_Adxl345_getRange(uint8_t *range);

void ControllerImpl_sampling_setFifoWatermark();
void ControllerImpl_sampling_clearFifoWatermark();
void ControllerImpl_sampling_setFifoOverflow();
void ControllerImpl_sampling_on5usTimerExpired();
void ControllerImpl_sampling_onSamplingStarted();
void ControllerImpl_sampling_onSamplingStopped();
void ControllerImpl_sampling_onSamplingAborted();
void ControllerImpl_sampling_onSamplingFinished();
void ControllerImpl_sampling_doForwardAccelerationBuffer(
    const struct Sampling_Acceleration *buffer, uint16_t bufferLen,
    uint16_t startIndex);
void ControllerImpl_sampling_onFifoOverflow();
void ControllerImpl_sampling_doEnableSensor();
void ControllerImpl_sampling_doDisableSensor();
void ControllerImpl_sampling_doFetchSensorAcceleration(
    struct Sampling_Acceleration *sample);
