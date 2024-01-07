/**
 * \file sampling.h
 *
 * Sampling module taking data from sensor's Fifo and monitoring interrupts.
 *
 * Takes monitors sensor interrupts (watermark, overrun), samples from sensor's
 * FiFo and forwards samples en-bloc.
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

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
void Sampling_start(uint16_t maxSamples);

/**
 * Stops sampling if not stopped so far.
 *
 * Called by:
 *   - sampling.c
 *   - TransportRx_Process(uint8_t *buffer, const uint32_t *length)
 */
void Sampling_stop();

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
int Sampling_fetchForward();

/**
 * Sets the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI2_IRQHandler(void)
 */
void Sampling_setFifoWatermark();

/**
 * Clears the FiFo watermark flag.
 *
 * Called by respective interrupt handler (on falling INT edge).
 * Handler: EXTI2_IRQHandler()
 */
void Sampling_clearFifoWatermark();

/**
 * Sets the FiFo overrun flag.
 *
 * Called by respective interrupt handler (on rising INT edge).
 * Handler: EXTI3_IRQHandler()
 */
void Sampling_setFifoOverflow();

/**
 * Clears the wait flag.
 *
 * Called by respective timer interrupt.
 * Handler: TIM3_IRQHandler()
 */
void Sampling_on5usTimerExpired();
