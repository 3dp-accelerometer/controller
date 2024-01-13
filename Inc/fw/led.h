/** \file led.h
 *
 * LED pin macros for convenient GPIO manipulation.
 *
 **/

#include "main.h"

#pragma once

/**
 * LED 0: PC13
 * @{
 */
#define USER_LED0_ON                                                           \
  HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_RESET)
#define USER_LED0_OFF                                                          \
  HAL_GPIO_WritePin(USER_LED0_GPIO_Port, USER_LED0_Pin, GPIO_PIN_SET)
#define USER_LED0_TOGGLE HAL_GPIO_TogglePin(USER_LED0_GPIO_Port, USER_LED0_Pin)
/// @}
