#pragma once
#include <stdbool.h>

extern bool DEVICE_REBOOT_DO_REBOOT;

void device_reboot_checkReboot();
void device_reboot_requestAsyncReboot();
