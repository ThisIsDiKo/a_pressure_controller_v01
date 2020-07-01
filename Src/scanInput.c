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
	uint8_t curInputWireState = 1;
	uint8_t prevInputWireState = 1;

	for(;;){
		curInputWireState = HAL_GPIO_ReadPin(WIRE_SENS_PORT, WIRE_SENS_PIN);
		if (curInputWireState != prevInputWireState){
			vTaskDelay(1 / portTICK_RATE_MS);
			curInputWireState = HAL_GPIO_ReadPin(WIRE_SENS_PORT, WIRE_SENS_PIN);
			if (curInputWireState != prevInputWireState){
				prevInputWireState = curInputWireState;
				if (!prevInputWireState){
					//storedOffsetmeanings
					//need to recalculate using featured sensors
//					controllerData.offsetPressure[0] = controllerState.filteredData[0];
//					controllerData.offsetPressure[1] = controllerState.filteredData[1];
//					controllerData.offsetPressure[2] = controllerState.filteredData[2];
//					controllerData.offsetPressure[3] = controllerState.filteredData[3];

//					mWrite_flash();

					controllerState.soundIndicationState = SEARCH_INDICATION;
					CMD_RF_ON;
					vTaskDelay(50 / portTICK_RATE_MS);

					HAL_UART_Transmit(&huart1, (uint8_t*) "AT+C001\r", 8, 0x2000);

					vTaskDelay(50 / portTICK_RATE_MS);
					CMD_RF_OFF;
				}
			}
		}
		vTaskDelay(5 / portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

