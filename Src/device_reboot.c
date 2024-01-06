#include "device_reboot.h"
#include <stm32f4xx_hal.h>

/**
 * Flag indicating a device reboot was requested.
 */
bool DEVICE_REBOOT_DO_REBOOT = false;

void device_reboot_checkReboot() {
  if (DEVICE_REBOOT_DO_REBOOT) {
    NVIC_SystemReset();
  }
}

void device_reboot_requestAsyncReboot() { DEVICE_REBOOT_DO_REBOOT = true; }
