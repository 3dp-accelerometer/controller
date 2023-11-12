#pragma once

#include <inttypes.h>
#include <stdbool.h>

/// Starts sampling of up to N samples if no sampling was started so far.
/// Set N=0 for indefinite sampling (until #sampling_stop() is called).
void sampling_start(uint16_t maxSamples);

/// Stops sampling if not stopped so far.
void sampling_stop();

/// Reads data from sensor if watermark is set and forwards via USB en-bloc.
/// Sends start, stop, finished, aborted and overflow USB messages accordingly.
/// \return
/// -EAGAIN if sampling is stopped,
/// -EOVERFLOW if FiFo OVL was detected;
/// ENODATA if all data was read (see #sampling_startN(uint16_t)),
/// 0 otherwise
int sampling_fetchForward();

/// Sets the FiFo watermark flag.
/// Normally called by respective interrupt handler (on rising INT edge).
void sampling_setFifoWatermark();

/// Clears the FiFo watermark flag.
/// Normally called by respective interrupt handler (on falling INT edge).
void sampling_clearFifoWatermark();

/// Sets the FiFo overrun flag.
/// Normally called by respective interrupt handler (on rising INT edge).
void sampling_setFifoOverflow();

/// Clears the wait flag; called by respective timer interrupt.
void on5usTimerExpired();
