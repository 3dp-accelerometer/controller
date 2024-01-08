/**
 * \file controller_reboot.h
 *
 * Implements scheduling a device reboot.
 */

#pragma once

/**
 * Resets device device if DEVICE_REBOOT_DO_REBOOT is set.
 *
 * Must be polled.
 */
void ControllerReboot_checkReboot();

/**
 * Requests reboot which is scheduled at a later time but ASAP.
 */
void ControllerReboot_requestAsyncReboot();
