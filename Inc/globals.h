/*
 * globals.h
 *
 *  Created on: 20 рту. 2019 у.
 *      Author: ADiKo
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include <string.h>

#define DEBUG_SERIAL	0

#define HALL_SENS_PORT							GPIOA
#define HALL_SENS_PIN							GPIO_PIN_12
#define WIRE_SENS_PORT							GPIOB
#define WIRE_SENS_PIN							GPIO_PIN_10
#define OVERCURRENT_PORT						GPIOB
#define OVERCURRENT_PIN							GPIO_PIN_11

#define BUZZER_PORT								GPIOC
#define BUZZER_PIN								GPIO_PIN_0

#define C1_UP_PORT								GPIOB
#define C1_UP_PIN								GPIO_PIN_12
#define C1_DOWN_PORT							GPIOB
#define C1_DOWN_PIN								GPIO_PIN_13
#define C2_UP_PORT								GPIOB
#define C2_UP_PIN								GPIO_PIN_14
#define C2_DOWN_PORT							GPIOB
#define C2_DOWN_PIN								GPIO_PIN_15
#define C3_UP_PORT								GPIOC
#define C3_UP_PIN								GPIO_PIN_6
#define C3_DOWN_PORT							GPIOC
#define C3_DOWN_PIN								GPIO_PIN_7
#define C4_UP_PORT								GPIOC
#define C4_UP_PIN								GPIO_PIN_8
#define C4_DOWN_PORT							GPIOC
#define C4_DOWN_PIN								GPIO_PIN_9

#define CMD_RF_PORT								GPIOA
#define CMD_RF_PIN								GPIO_PIN_11

#define C1_UP_ON								HAL_GPIO_WritePin(C1_UP_PORT, 	C1_UP_PIN, GPIO_PIN_SET)
#define C1_UP_OFF								HAL_GPIO_WritePin(C1_UP_PORT, 	C1_UP_PIN, GPIO_PIN_RESET)
#define C1_DOWN_ON								HAL_GPIO_WritePin(C1_DOWN_PORT, C1_DOWN_PIN, GPIO_PIN_SET)
#define C1_DOWN_OFF								HAL_GPIO_WritePin(C1_DOWN_PORT, C1_DOWN_PIN, GPIO_PIN_RESET)

#define C2_UP_ON								HAL_GPIO_WritePin(C2_UP_PORT, C2_UP_PIN, GPIO_PIN_SET)
#define C2_UP_OFF								HAL_GPIO_WritePin(C2_UP_PORT, C2_UP_PIN, GPIO_PIN_RESET)
#define C2_DOWN_ON								HAL_GPIO_WritePin(C2_DOWN_PORT, C2_DOWN_PIN, GPIO_PIN_SET)
#define C2_DOWN_OFF								HAL_GPIO_WritePin(C2_DOWN_PORT, C2_DOWN_PIN, GPIO_PIN_RESET)

#define C3_UP_ON								HAL_GPIO_WritePin(C3_UP_PORT, C3_UP_PIN, GPIO_PIN_SET)
#define C3_UP_OFF								HAL_GPIO_WritePin(C3_UP_PORT, C3_UP_PIN, GPIO_PIN_RESET)
#define C3_DOWN_ON								HAL_GPIO_WritePin(C3_DOWN_PORT, C3_DOWN_PIN, GPIO_PIN_SET)
#define C3_DOWN_OFF								HAL_GPIO_WritePin(C3_DOWN_PORT, C3_DOWN_PIN, GPIO_PIN_RESET)

#define C4_UP_ON								HAL_GPIO_WritePin(C4_UP_PORT, C4_UP_PIN, GPIO_PIN_SET)
#define C4_UP_OFF								HAL_GPIO_WritePin(C4_UP_PORT, C4_UP_PIN, GPIO_PIN_RESET)
#define C4_DOWN_ON								HAL_GPIO_WritePin(C4_DOWN_PORT, C4_DOWN_PIN, GPIO_PIN_SET)
#define C4_DOWN_OFF								HAL_GPIO_WritePin(C4_DOWN_PORT, C4_DOWN_PIN, GPIO_PIN_RESET)

#define CMD_RF_OFF								HAL_GPIO_WritePin(CMD_RF_PORT, CMD_RF_PIN, GPIO_PIN_SET)
#define CMD_RF_ON								HAL_GPIO_WritePin(CMD_RF_PORT, CMD_RF_PIN, GPIO_PIN_RESET)

#define MAX_COMMAND_LENGTH						64
#define COMMAND_QUEUE_SIZE						2


#define SENS_1						0
#define SENS_2						1
#define SENS_3						2
#define SENS_4						3

#define CURRENT_SENS				4

#define PRESSURE_ACCURACY			75

#define ADC_DATA_PERIOD			50

#define SETTINGS_FLASH_PAGE_ADDR	0x0801F800 //TODO: check address

#define MAX_ANALYZE_TRIES			7

#define VIEW_SINGLE   1 //NO USE
#define VIEW_0_2   2
#define VIEW_1_1   3
#define VIEW_1_2   4
#define VIEW_2_1   5
#define VIEW_2_2   6

#endif /* GLOBALS_H_ */
