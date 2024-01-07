/**
 * \file device_reboot.h
 *
 * Implements scheduling a device reboot.
 */

#pragma once
#include <stdbool.h>

/**
 * Resets device device if DEVICE_REBOOT_DO_REBOOT is set.
 *
 * Must be polled.
 */
void DeviceReboot_checkReboot();

/**
 * Requests reboot which is scheduled at a later time but ASAP.
 */
void DeviceReboot_requestAsyncReboot();
