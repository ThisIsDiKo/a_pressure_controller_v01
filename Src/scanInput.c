/*
 * scanInput.c
 *
 *  Created on: 16 сент. 2019 г.
 *      Author: ADiKo
 */

#include "globals.h"
#include "structures.h"

#include "scanInput.h"
#include "controllerUtils.h"
#include "flashFunctions.h"

extern UART_HandleTypeDef huart1;
extern enum IndicationState indicationState;

void xScanInputTask(void* arguments){
	uint8_t prevInputHallState = 1;
	uint8_t curInputHallState = 1;
	uint8_t curInputWireState = 1;
	uint8_t prevInputWireState = 1;

	uint8_t prevOverrcurrentState = 0;
	uint8_t curOvercurrentState = 0;

	for(;;){
		curInputHallState = HAL_GPIO_ReadPin(HALL_SENS_PORT, HALL_SENS_PIN);
		curInputWireState = HAL_GPIO_ReadPin(WIRE_SENS_PORT, WIRE_SENS_PIN);

		if ((curInputHallState != prevInputHallState) || (curInputWireState != prevInputWireState)){

			vTaskDelay(1 / portTICK_RATE_MS);
			curInputHallState = HAL_GPIO_ReadPin(HALL_SENS_PORT, HALL_SENS_PIN);
			curInputWireState = HAL_GPIO_ReadPin(WIRE_SENS_PORT, WIRE_SENS_PIN);

			if ((curInputHallState != prevInputHallState) || (curInputWireState != prevInputWireState)){

				prevInputHallState = curInputHallState;
				prevInputWireState = curInputWireState;

				if ((!prevInputHallState) || (!prevInputWireState)){

					//storedOffsetmeanings
					//need to recalculate using featured sensors
					controllerData.offsetPressure[0] = controllerState.filteredData[0];
					controllerData.offsetPressure[1] = controllerState.filteredData[1];
					controllerData.offsetPressure[2] = controllerState.filteredData[2];
					controllerData.offsetPressure[3] = controllerState.filteredData[3];
					mWrite_flash();

					controllerState.soundIndicationState = SEARCH_INDICATION;
					CMD_RF_ON;
					vTaskDelay(50 / portTICK_RATE_MS);

					HAL_UART_Transmit(&huart1, (uint8_t*) "AT+C001\r", 8, 0x2000);

					vTaskDelay(50 / portTICK_RATE_MS);
					CMD_RF_OFF;
				}
			}
		}

		curOvercurrentState = HAL_GPIO_ReadPin(OVERCURRENT_PORT, OVERCURRENT_PIN);

		if (curOvercurrentState != prevOverrcurrentState){

			vTaskDelay(1 / portTICK_RATE_MS);
			curOvercurrentState = HAL_GPIO_ReadPin(OVERCURRENT_PORT, OVERCURRENT_PIN);

			if (curOvercurrentState != prevOverrcurrentState){

				prevOverrcurrentState = curOvercurrentState;

				if (prevOverrcurrentState){
					C1_UP_OFF;
					C1_DOWN_OFF;
					C2_UP_OFF;
					C2_DOWN_OFF;
					C3_UP_OFF;
					C3_DOWN_OFF;
					C4_UP_OFF;
					C4_DOWN_OFF;

					controllerState.errorStatus |= (1 << STATUS_ERROR_OVERCURRENT);
					controllerState.pressureCompensation = COMPENSATION_OFF;

					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
					vTaskDelay(100);
					HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
					vTaskDelay(200);
					HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
					vTaskDelay(100);
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
					vTaskDelay(200);
					HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
					vTaskDelay(100);
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
					vTaskDelay(200);
					HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
					vTaskDelay(100);
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
					vTaskDelay(200);
					HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
					vTaskDelay(100);
					HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);



					C1_UP_OFF;
					C1_DOWN_OFF;
					C2_UP_OFF;
					C2_DOWN_OFF;
					C3_UP_OFF;
					C3_DOWN_OFF;
					C4_UP_OFF;
					C4_DOWN_OFF;
				}
			}
		}
		vTaskDelay(5 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

