#include "fw/controller_reboot.h"
#include <stdbool.h>
#include <stm32f4xx_hal.h>

/**
 * Flag indicating a device reboot was requested.
 *
 * \see ControllerReboot_requestAsyncReboot()
 */
static bool Controller_rebootRequested = false;

void ControllerReboot_checkReboot() {
  if (Controller_rebootRequested) {
    NVIC_SystemReset();
  }
}

void ControllerReboot_requestAsyncReboot() {
  Controller_rebootRequested = true;
}
