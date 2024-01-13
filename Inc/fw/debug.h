/** \file debug.h
*
* Debug pin macros for convenient GPIO manipulation.
*
**/

#include "main.h"

#pragma once

/**
 * DEBUG 0: Pin PA1
 * @{
 */
#define USER_DEBUG0_HIGH   HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_SET)
#define USER_DEBUG0_LOW    HAL_GPIO_WritePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin, GPIO_PIN_RESET)
#define USER_DEBUG0_TOGGLE HAL_GPIO_TogglePin(USER_DEBUG1_GPIO_Port, USER_DEBUG1_Pin)
/// @}

/**
 * DEBUG 1: Pin PB0
 * @{
 */
#define USER_DEBUG1_HIGH   HAL_GPIO_WritePin(USER_DEBUG1_GPIO_Port, USER_DEBUG1_Pin, GPIO_PIN_SET)
#define USER_DEBUG1_LOW    HAL_GPIO_WritePin(USER_DEBUG1_GPIO_Port, USER_DEBUG1_Pin, GPIO_PIN_RESET)
#define USER_DEBUG1_TOGGLE HAL_GPIO_TogglePin(USER_DEBUG0_GPIO_Port, USER_DEBUG0_Pin)
/// @}
