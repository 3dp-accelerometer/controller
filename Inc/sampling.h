#pragma once

#include <inttypes.h>
#include <stdbool.h>

void sampling_start();
void sampling_startN(uint16_t maxSamples);
void sampling_stop();
int sampling_fetchForward();

void sampling_setFifoWatermark();
void sampling_clearFifoWatermark();

void sampling_setFifoOverflow();
void sampling_clearFifoOverflow();
bool sampling_hasFifoOverflow();
