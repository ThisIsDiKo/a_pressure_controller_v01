/*
 * analyze.c
 *
 *  Created on: 16 ����. 2019 �.
 *      Author: ADiKo
 */

#include "globals.h"
#include "structures.h"
#include "analyze.h"
#include "flashFunctions.h"

extern xSemaphoreHandle xPressureCompensationSemaphore;
extern UART_HandleTypeDef huart1;

extern GPIO_TypeDef *UP_PORT[4];
extern uint32_t UP_PIN[4];
extern GPIO_TypeDef *DOWN_PORT[4];
extern uint32_t DOWN_PIN[4];

extern char message[128];
extern uint8_t messageLength;


void xAnalyzeTask(void *arguments){
	portBASE_TYPE xStatus;

	uint8_t i = 0;
	int16_t deltaPressure = 0;

	int32_t impTime[4] = {0, 1, 2, 3};
	float impCoeff[4] = {0.0,0.0,0.0,0.0};
	uint16_t startPressure[4];
	uint32_t dCounter = 0;
	uint8_t stopImp = 0;
	uint32_t impCounter = 0;
	uint8_t numOfAxles = 0;
	uint8_t numOfWays[2] = {0, 0};
	uint8_t axleCounter = 0;
	uint8_t wayCounter = 0;

	int8_t pressIsLower[4] = {0};

	int8_t goodPressCounter = 0;


	xStatus = xSemaphoreTake(xPressureCompensationSemaphore, portMAX_DELAY);
	for(;;){
		xStatus = xSemaphoreTake(xPressureCompensationSemaphore, portMAX_DELAY);
		if (xStatus == pdPASS){

			if (controllerState.airSystemType == RECEIVER){
				//-------- check for number of tries -----------------
				controllerState.analyzeState = COMPENSATION_STATE_FREE;
				if (controllerState.numberOfTries >= MAX_ANALYZE_TRIES){
					controllerState.numberOfTries = 0;
					controllerState.pressureCompensation = COMPENSATION_OFF;

					#if DEBUG_SERIAL
						messageLength = sprintf(message, "[INFO] exit by tries\n");
						HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
					#endif

				}
				else{
					controllerState.numberOfTries += 1;
				}


				//-------- looking at pressure delta -----------------
				for (i = 0; i < 4; i++){
					startPressure[i] = controllerState.filteredData[i];
					deltaPressure = controllerState.nessPressure[i] - controllerState.filteredData[i];
					deltaPressure = abs(deltaPressure);

					if (deltaPressure > controllerState.analyzeAccuracy){
						if (controllerState.nessPressure[i] > controllerState.filteredData[i])
							pressIsLower[i] = 1;
						else
							pressIsLower[i] = 0;
						controllerState.analyzeState = COMPENSATION_STATE_WORKING;
					}
					else{
						pressIsLower[i] = -1;
					}
				}

				//-------- if everything is OK stop compensation -----------------
				if (controllerState.analyzeState == COMPENSATION_STATE_FREE){
					controllerState.pressureCompensation = COMPENSATION_OFF;
					impTime[0] = 0;
					impTime[1] = 0;
					impTime[2] = 0;
					impTime[3] = 0;
					controllerState.numberOfTries = 0;

					mWrite_flash();
					continue;
				}


				switch (controllerState.waysType){
					case 1:{ // Single Way
						numOfAxles = 1;
						numOfWays[0] = 1;
						numOfWays[1] = 0;
						break;
					}
					case 2:{ // TWO WAYS ONE AXLE
						numOfAxles = 1;
						numOfWays[0] = 2;
						numOfWays[1] = 0;
						break;
					}
					case 3:{ //TWO WAYS TWO AXLE
						numOfAxles = 2;
						numOfWays[0] = 1;
						numOfWays[1] = 1;
						break;
					}
					case 4:{ //THREE WAYS TWO FRONT AXLES
						numOfAxles = 2;
						numOfWays[0] = 2;
						numOfWays[1] = 1;
						break;
					}
					case 5:{ //THREE WAYS SINGLE FRONT AXLES
						numOfAxles = 2;
						numOfWays[0] = 1;
						numOfWays[1] = 2;
						break;
					}
					case 6:{ //FOUR WAYS
						numOfAxles = 2;
						numOfWays[0] = 2;
						numOfWays[1] = 2;
						break;
					}
					default:{
						controllerState.pressureCompensation = COMPENSATION_OFF;
						continue;
					}

				}

				//calculate impulse
				#if DEBUG_SERIAL
					messageLength = sprintf(message, "[INFO] ---IMP DATA---\n");
					HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
				#endif


				for (axleCounter = 0; axleCounter < numOfAxles; axleCounter++){
					for (wayCounter = 0; wayCounter < numOfWays[axleCounter]; wayCounter++){
						i = axleCounter*numOfWays[0] + wayCounter;
						deltaPressure = controllerState.nessPressure[i] - controllerState.filteredData[i];
						if (pressIsLower[i] == 1){

							impTime[i] = (int32_t)(controllerData.impUpCoeff[i] * (float)deltaPressure);

							#if DEBUG_SERIAL
								messageLength = sprintf(message, "[INFO] %d: up %ld\n", i, impTime[i]);
								HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
							#endif

							if (impTime[i] < 0) impTime[i] = 0;
							else if (impTime[i] == 0) impTime[i] = 1000;
							else if (impTime[i] > 10000) impTime[i] = 10000;
							else if (impTime[i] > 30000) impTime[i] = 1000;
						}
						else if (pressIsLower[i] == 0){
							impTime[i] = (int32_t)(controllerData.impDownCoeff[i] * (float)deltaPressure);

							#if DEBUG_SERIAL
								messageLength = sprintf(message, "[INFO] %d: down %ld\n", i,  impTime[i]);
								HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
							#endif

							if (impTime[i] < 0) impTime[i] = 0;
							else if (impTime[i] == 0) impTime[i] = 500;
							else if (impTime[i] > 10000) impTime[i] = 10000;
							else if (impTime[i] > 30000) impTime[i] = 500;
						}
						else{
							impTime[i] = 0;
						}
					}

					if (controllerState.pressureCompensation == COMPENSATION_OFF){
						for (i = 0; i < 4; i++){
							HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_RESET);
							HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_RESET);
						}
						impTime[0] = 0;
						impTime[1] = 0;
						impTime[2] = 0;
						impTime[3] = 0;
						controllerState.numberOfTries = 0;
						break;
					}

					for (wayCounter = 0; wayCounter < numOfWays[axleCounter]; wayCounter++){
						//i = axleCounter + axleCounter*numOfWays[0] + wayCounter;
						i = axleCounter*numOfWays[0] + wayCounter;

						#if DEBUG_SERIAL
							messageLength = sprintf(message, "[INFO] %d: time %ld\n", i,  impTime[i]);
							HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
						#endif

						if (impTime[i] > 0){
							if (pressIsLower[i] == 1){
								HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_SET);
							}
							else if (pressIsLower[i] == 0){
								HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_SET);
							}
						}
					}

					#if DEBUG_SERIAL
						messageLength = sprintf(message, "[INFO] ac %d, nw0 %d, nw1 %d\n", axleCounter, numOfWays[0], numOfWays[1]);
						HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
					#endif

					impCounter = xTaskGetTickCount();
					while(1){
						vTaskDelay(20);
						dCounter = xTaskGetTickCount() - impCounter;

						stopImp = 0;
						for (wayCounter = 0; wayCounter < numOfWays[axleCounter]; wayCounter++){
							i = axleCounter*numOfWays[0] + wayCounter;
							if(dCounter > impTime[i]){
								HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_RESET);
								HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_RESET);
								stopImp++;
							}
						}
						if (stopImp >= numOfWays[axleCounter]){
							break;
						}
					}
					vTaskDelay(1000);
				} //stepCounter
				if (controllerState.pressureCompensation == COMPENSATION_OFF){
					continue;
				}
//here starts common code


				for (i = 0 ; i < 4; i++){
					if (impTime[i] > 500){
						deltaPressure = controllerState.filteredData[i] - startPressure[i];
						deltaPressure = abs(deltaPressure);
						if (deltaPressure < 10){
							pressIsLower[i] = -1;

							#if DEBUG_SERIAL
								messageLength = sprintf(message, "[ERROR] %d valve %d\t%d\t%d\t%ld\n", i, controllerState.nessPressure[i], startPressure[i], controllerState.filteredData[i], impTime[i]);
								HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
							#endif

						}
					}
				}

				#if DEBUG_SERIAL
					messageLength = sprintf(message, "[INFO] Results\n");
					HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
				#endif

				for (i = 0 ; i < 4; i++){
					if (pressIsLower[i] >=0){
						deltaPressure = controllerState.filteredData[i] - startPressure[i];
						impCoeff[i] = (float)impTime[i] / (float) deltaPressure;
						if (pressIsLower[i] == 1){
							if (impCoeff[i] >= 0.0)
								controllerData.impUpCoeff[i] = impCoeff[i];
						}
						else if (pressIsLower[i] == 0){
							if (impCoeff[i] <= 0.0)
								controllerData.impDownCoeff[i] = impCoeff[i];
						}

						#if DEBUG_SERIAL
							messageLength = sprintf(message, "\t%d: %d\t%d\t%d\t%ld\t%d\t%d\n", i, controllerState.nessPressure[i], startPressure[i], controllerState.filteredData[i], impTime[i],(int)controllerData.impUpCoeff[i],(int)controllerData.impDownCoeff[i]);
							HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
						#endif

					}
				}
				//mWrite_flash();
			}
			else{
				//TODO: compressor system
				switch (controllerState.waysType){
					case 1:{ // Single Way
						numOfAxles = 1;
						break;
					}
					case 2:{ // TWO WAYS ONE AXLE
						numOfAxles = 2;
						break;
					}
					case 3:{ //TWO WAYS TWO AXLE
						numOfAxles = 2;
						break;
					}
					case 4:{ //THREE WAYS TWO FRONT AXLES
						numOfAxles = 3;
						break;
					}
					case 5:{ //THREE WAYS SINGLE FRONT AXLE
						numOfAxles = 3;
						break;
					}
					case 6:{ //FOUR WAYS
						numOfAxles = 4;
						break;
					}
					default:{
						controllerState.pressureCompensation = COMPENSATION_OFF;
						continue;
					}

				}

				controllerState.analyzeState = COMPENSATION_STATE_FREE;
				if (controllerState.numberOfTries >= MAX_ANALYZE_TRIES){
					controllerState.numberOfTries = 0;
					controllerState.pressureCompensation = COMPENSATION_OFF;

					#if DEBUG_SERIAL
						messageLength = sprintf(message, "[INFO] exit by tries\n");
						HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
					#endif

					break;
				}
				else{
					controllerState.numberOfTries += 1;
				}

				goodPressCounter = numOfAxles;
				while(1){
					for (i = 0; i < numOfAxles; i++){
						startPressure[i] = controllerState.filteredData[i];
						deltaPressure = controllerState.nessPressure[i] - controllerState.filteredData[i];
						deltaPressure = abs(deltaPressure);

						if (deltaPressure > controllerState.analyzeAccuracy){
							if (controllerState.nessPressure[i] > controllerState.filteredData[i]){
								pressIsLower[i] = 1;
								HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_SET);
								HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_RESET);
							}
							else{
								pressIsLower[i] = 0;
								HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_RESET);
								HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_SET);
							}
							controllerState.analyzeState = COMPENSATION_STATE_WORKING;
						}
						else{
							pressIsLower[i] = -1;
							goodPressCounter -= 1;
							HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_RESET);
							HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_RESET);
						}
					}

					#if DEBUG_SERIAL
						messageLength = sprintf(message, "[INFO] gP counter %d\n", goodPressCounter);
						HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
					#endif

					if(goodPressCounter == 0) break;

					vTaskDelay(2000);

					controllerState.compressorWorkTime += 1000;
					if (controllerState.compressorWorkTime > 600000){
						controllerState.analyzeState = COMPENSATION_STATE_FREE;
						controllerState.pressureCompensation = COMPENSATION_OFF;
						controllerState.numberOfTries = 0;
						controllerState.compressorWorkTime = 0;

						#if DEBUG_SERIAL
							messageLength = sprintf(message, "[ERROR] exit by compressor time\n");
							HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
						#endif

						break;
					}
				}

				for (i = 0; i < numOfAxles; i++){
					HAL_GPIO_WritePin(DOWN_PORT[i], DOWN_PIN[i], GPIO_PIN_RESET);
					HAL_GPIO_WritePin(UP_PORT[i], UP_PIN[i], GPIO_PIN_RESET);
				}
				vTaskDelay(2000);
			}
		}
	}
}
/*
 * analyze.c
 *
 *  Created on: 4 ���. 2019 �.
 *      Author: ADiKo
 */

