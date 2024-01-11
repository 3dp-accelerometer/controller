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

struct Sampling_Handle;

/**
 * Starts sampling of up to N samples if no sampling was started so far.
 *
 * Set N=0 for indefinite sampling (until Sampling_stop() is called).
 *
 * Called by:
 *   - sampling.c
 *   - TransportRx_Process(uint8_t *buffer, uint16_t length)
 *
 * \param handle module internal state and device dependent pimpl
 * \param maxSamples amount of samples requested, infinite if 0
 */
void Sampling_start(struct Sampling_Handle *handle, uint16_t maxSamples);

/**
 * Stops sampling if not stopped so far.
 *
 * Called by:
 *   - sampling.c
 *   - TransportRx_Process(uint8_t *buffer, uint16_t length)
 *
 * \param handle module internal state and device dependent pimpl
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
 *
 * \param handle module internal state and device dependent pimpl
 */
int Sampling_fetchForward(struct Sampling_Handle *handle);

/**
 * Sets the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI2_IRQHandler(void)
 *
 * \param handle module internal state and device dependent pimpl
 */
void Sampling_setFifoWatermark(struct Sampling_Handle *handle);

/**
 * Clears the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on falling INT edge).
 * Handler: EXTI2_IRQHandler()
 *
 * \param handle module internal state and device dependent pimpl
 */
void Sampling_clearFifoWatermark(struct Sampling_Handle *handle);

/**
 * Sets the FiFo overrun flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI3_IRQHandler()
 *
 * \param handle module internal state and device dependent pimpl
 */
void Sampling_setFifoOverflow(struct Sampling_Handle *handle);

/**
 * Clears the wait flag.
 *
 * Called by respective timer interrupt.
 * Handler: TIM3_IRQHandler()
 *
 * \param handle module internal state and device dependent pimpl
 */
void Sampling_on5usTimerExpired(struct Sampling_Handle *handle);
