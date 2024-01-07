#include "fw/device_reboot.h"
#include <stm32f4xx_hal.h>

/**
 * Flag indicating a device reboot was requested.
 *
 * \see DeviceReboot_requestAsyncReboot()
 */
static bool DEVICE_REBOOT_DO_REBOOT = false;

void DeviceReboot_checkReboot() {
  if (DEVICE_REBOOT_DO_REBOOT) {
    NVIC_SystemReset();
  }
}

void DeviceReboot_requestAsyncReboot() { DEVICE_REBOOT_DO_REBOOT = true; }
