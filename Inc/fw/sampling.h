/**
 * \file sampling.h
 *
 * Sampling module taking data from sensor's Fifo and monitoring interrupts.
 *
 * Takes monitors sensor interrupts (watermark, overrun), samples from sensor's
 * FiFo and forwards samples en-bloc.
 */

#pragma once

#include <adxl345.h>
#include <adxl345_transport_types.h>
#include <inttypes.h>
#include <stdbool.h>

/**
 * Configures how many samples can be maximally read at once from the sensor.
 *
 * Must be less than or equal watermark level to not read beyond buffered FiFo.
 */
#define NUM_SAMPLES_READ_AT_ONCE ADXL345_WATERMARK_LEVEL

#define MYSTRINGIZE0(A) #A
#define MYSTRINGIZE(A) MYSTRINGIZE0(A)

static_assert(
    NUM_SAMPLES_READ_AT_ONCE <= ADXL345_WATERMARK_LEVEL,
    "maximum allowed read-at-once: " MYSTRINGIZE(ADXL345_WATERMARK_LEVEL));

static_assert(ADXL345_WATERMARK_LEVEL > 0,
              "minimum required watermark level: 1");
#undef MYSTRINGIZE
#undef MYSTRINGIZE0

/**
 * Internal module state.
 */
struct Sampling_Handle {
  volatile uint16_t maxSamples;
  volatile bool doStart;
  volatile bool doStop;
  volatile bool isStarted;
  volatile bool waitFor5usTimer;
  struct Adxl345Transport_Acceleration rxBuffer[NUM_SAMPLES_READ_AT_ONCE];
  volatile bool isFifoOverflowSet;
  volatile bool isFifoWatermarkSet;
  int transactionsCount;
};

#define SAMPLING_DECLARE_HANDLE(HANDLE_NAME)                                   \
  struct Sampling_Handle HANDLE_NAME = {                                       \
      .maxSamples = 0,                                                         \
      .doStart = false,                                                        \
      .doStop = false,                                                         \
      .isStarted = false,                                                      \
      .waitFor5usTimer = false,                                                \
      .rxBuffer = {{.x = 0, .y = 0, .z = 0}},                                  \
      .isFifoOverflowSet = false,                                              \
      .isFifoWatermarkSet = false,                                             \
      .transactionsCount = 0,                                                  \
  };

/**
 * Starts sampling of up to N samples if no sampling was started so far.
 *
 * Set N=0 for indefinite sampling (until Sampling_stop() is called).
 *
 * Called by:
 *   - sampling.c
 *   - TransportRx_Process(uint8_t *buffer, const uint32_t *length)
 *
 * \param maxSamples amount of samples requested, infinite if 0
 */
void Sampling_start(struct Sampling_Handle *handle, uint16_t maxSamples);

/**
 * Stops sampling if not stopped so far.
 *
 * Called by:
 *   - sampling.c
 *   - TransportRx_Process(uint8_t *buffer, const uint32_t *length)
 */
void Sampling_stop(struct Sampling_Handle *handle);

/**
 * Reads data from sensor if watermark is set and forwards to USB en-bloc.
 *
 * Sends start, stop, finished, aborted and overflow USB messages accordingly.
 * Called by main().
 *
 * \return
 *   - -EAGAIN if sampling is stopped
 *   - -EOVERFLOW if FiFo OVL was detected
 *   - ENODATA if all data was read (see sampling_startN(uint16_t))
 *   - 0 otherwise
 */
int Sampling_fetchForward(struct Sampling_Handle *handle);

/**
 * Sets the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI2_IRQHandler(void)
 */
void Sampling_setFifoWatermark(struct Sampling_Handle *handle);

/**
 * Clears the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on falling INT edge).
 * Handler: EXTI2_IRQHandler()
 */
void Sampling_clearFifoWatermark(struct Sampling_Handle *handle);

/**
 * Sets the FiFo overrun flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI3_IRQHandler()
 */
void Sampling_setFifoOverflow(struct Sampling_Handle *handle);

/**
 * Clears the wait flag.
 *
 * Called by respective timer interrupt.
 * Handler: TIM3_IRQHandler()
 */
void Sampling_on5usTimerExpired(struct Sampling_Handle *handle);
