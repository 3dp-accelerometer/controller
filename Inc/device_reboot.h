/**
 * \file device_reboot.h
 *
 * Allows scheduling device reboot.
 */
#pragma once
#include <stdbool.h>

// todo: remove
extern bool DEVICE_REBOOT_DO_REBOOT;

/**
 * Hook to reboot device if DEVICE_REBOOT_DO_REBOOT is set.
 *
 * Must be polled.
 */
void device_reboot_checkReboot();

/**
 * Requests reboot which is scheduled at a later time but ASAP.
 */
void device_reboot_requestAsyncReboot();
