/*
 * processCommand.c
 *
 *  Created on: 16 сент. 2019 г.
 *      Author: ADiKo
 */
#include "globals.h"
#include "structures.h"

#include "processCommand.h"
#include "flashFunctions.h"
#include "controllerUtils.h"

extern UART_HandleTypeDef huart1;


extern xQueueHandle xRecCommandQueue;

extern char message[128];
extern uint8_t messageLength;

void xProcessCommandTask(void* arguments){
	portBASE_TYPE xStatus;
	uint8_t command[MAX_COMMAND_LENGTH] = {0};

	uint16_t id = 0;
	uint8_t i = 0;
	char co = 0;
	char outputState = 0;
	uint16_t channel = 0;

	char prevOutputState = 0;
	char systemType = 0;
	//char statusByte = 0;
	char waysType;
	char accuracy;
	uint8_t successCounter = 0;

	for(;;){
		xStatus = xQueueReceive(xRecCommandQueue, command, portMAX_DELAY);
		if (xStatus == pdPASS){

			controllerState.lastTimeCommand = 0;

			switch(command[0]){
			case 'o':{

				sscanf((char*)command, "o,%hu,\n", &id);
				if (id == controllerState.serverUID){

					controllerState.status = 0;
					if (controllerState.pressureCompensation == COMPENSATION_ON){
						controllerState.status = 0x01;
					}

					//valve overcurrent error
					if (controllerState.errorStatus & (1 << STATUS_ERROR_OVERCURRENT)){
						controllerState.status |= 0x02;
					}

					//pressure valve error
					if (controllerState.errorStatus & (1 << STATUS_ERROR_VALVE)){
						controllerState.status |= 0x03;
					}

					//added statusByte for best indication
					messageLength = sprintf(message, "o,%hu,%hu,%hu,%hu,%hu,%c,\n", controllerData.clientID,
																					controllerState.filteredData[SENS_1],
																					controllerState.filteredData[SENS_2],
																					controllerState.filteredData[SENS_3],
																					controllerState.filteredData[SENS_4],
																					controllerState.status);
					HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
				}
				break;
			}
				case 'm':{
					sscanf((char*)command, "m,%hu,%c,%c,\n", &id, &co, &outputState);
					outputState = command[10];
					if (id == controllerState.serverUID){

						controllerState.status = 0;
						if (controllerState.pressureCompensation == COMPENSATION_ON){
							controllerState.status = 0x01;
						}

						//valve overcurrent error
						if (controllerState.errorStatus & (1 << STATUS_ERROR_OVERCURRENT)){
							//controllerState.status |= 0x02;
							messageLength = sprintf(message, "w,%hu,\n",controllerData.clientID);
							HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
							controllerState.errorStatus = 0;
							break;
						}

						//pressure valve error
						if (controllerState.errorStatus & (1 << STATUS_ERROR_VALVE)){
							//controllerState.status |= 0x03;
							messageLength = sprintf(message, "q,%hu,0,\n",controllerData.clientID);
							HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
							controllerState.errorStatus = 0;
							break;
						}

						//added statusByte for best indication
						messageLength = sprintf(message, "m,%hu,%hu,%hu,%hu,%hu,%c,%c,%c,\n", 	controllerData.clientID,
																								controllerState.filteredData[SENS_1],
																								controllerState.filteredData[SENS_2],
																								controllerState.filteredData[SENS_3],
																								controllerState.filteredData[SENS_4],
																								controllerState.status,
																								controllerState.errorByte,
																								controllerState.errorMeaningByte);
						HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);

						//if (outputState != prevOutputState && controllerState.errorStatus == STATUS_NORMAL){
						if (outputState != prevOutputState && controllerState.errorStatus != STATUS_ERROR_OVERCURRENT){

							controllerState.pressureCompensation = COMPENSATION_OFF;

							if (outputState & 0b00000001) 	C1_UP_ON;
							else 				   			C1_UP_OFF;
							if (outputState & 0b00000010) 	C1_DOWN_ON;
							else 				   			C1_DOWN_OFF;

							if (outputState & 0b00000100) 	C2_UP_ON;
							else 				   			C2_UP_OFF;
							if (outputState & 0b00001000) 	C2_DOWN_ON;
							else 				   			C2_DOWN_OFF;

							if (outputState & 0b00010000) 	C3_UP_ON;
							else 				   			C3_UP_OFF;
							if (outputState & 0b00100000) 	C3_DOWN_ON;
							else 				   			C3_DOWN_OFF;

							if (outputState & 0b01000000) 	C4_UP_ON;
							else 				   			C4_UP_OFF;
							if (outputState & 0b10000000) 	C4_DOWN_ON;
							else 				   			C4_DOWN_OFF;

							prevOutputState = outputState;
						}
					}
					break;
				}
				case 's':{
					if (command[1] == 'x'){
						sscanf((char*)command, "sx,%hu,\n", &id);
						if (id == controllerState.serverUID){
							controllerState.pressureCompensation = COMPENSATION_OFF;
						}
					}
					else if (command[1] == ','){
						sscanf((char*)command, "s,%hu,%hu,%hu,%hu,%hu,%c,%c,%c,\n", &id, 	&controllerState.nessPressure[0],
																						&controllerState.nessPressure[1],
																						&controllerState.nessPressure[2],
																						&controllerState.nessPressure[3],
																						&systemType,
																						&waysType,
																						&accuracy);
						if (id == controllerState.serverUID && controllerState.errorStatus == STATUS_NORMAL){
							if (systemType == '1'){ // air system choice
								controllerState.airSystemType = COMPRESSOR; // for compressor
								controllerState.compressorWorkTime = 0;
							}
							else{
								controllerState.airSystemType = RECEIVER;
							}

							for(i = 0; i < 4; i++){
								if (controllerState.nessPressure[i] > 4000) controllerState.nessPressure[i] = controllerState.filteredData[i];
							}

							controllerState.waysType = waysType - '0';

							if (accuracy == '1'){
								controllerState.analyzeAccuracy = 30;
							}
							else if (accuracy == '2'){
								controllerState.analyzeAccuracy = 60;
							}
							else if (accuracy == '3'){
								controllerState.analyzeAccuracy = 90;
							}
							else{
								controllerState.analyzeAccuracy = 25;
							}

							if (controllerState.waysType > 6 || controllerState.waysType < 2){

								#if DEBUG_SERIAL
									messageLength = sprintf(message, "[ERROR] wrong waystype\n");
									HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
								#endif

								continue;
							}

							successCounter = 0;
							if(controllerState.waysType == VIEW_2_2){
								for(i = 0; i < 4; i++){
									if (controllerState.filteredData[i] == 0 || controllerState.nessPressure[i] == 0){
										//TODO: maybe make inform about sensors
										continue;
									}
									successCounter += 1;
								}
								if (successCounter != 4){
									#if DEBUG_SERIAL
										messageLength = sprintf(message, "[ERROR] wrong sens\n");
										HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
									#else
										messageLength = sprintf(message, "t,%hu,\n", 	controllerData.clientID);
										HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
									#endif
									continue;
								}
							}
							else if(controllerState.waysType == VIEW_2_1 || controllerState.waysType == VIEW_1_2){
								for(i = 0; i < 3; i++){
									if (controllerState.filteredData[i] == 0 || controllerState.nessPressure[i] == 0){
										//TODO: maybe make inform about sensors
										continue;
									}
									successCounter += 1;
								}
								if (successCounter != 3){
									#if DEBUG_SERIAL
										messageLength = sprintf(message, "[ERROR] wrong sens\n");
										HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
									#else
										messageLength = sprintf(message, "t,%hu,\n", 	controllerData.clientID);
										HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
									#endif
									continue;
								}
							}
							else if(controllerState.waysType == VIEW_1_1 || controllerState.waysType == VIEW_0_2){
								for(i = 0; i < 2; i++){
									if (controllerState.filteredData[i] == 0 || controllerState.nessPressure[i] == 0){
										//TODO: maybe make inform about sensors
										continue;
									}
									successCounter += 1;
								}
								if (successCounter != 2){
										#if DEBUG_SERIAL
											messageLength = sprintf(message, "[ERROR] wrong sens\n");
											HAL_UART_Transmit(&huart1, (uint8_t*)message, messageLength, 0xFFFF);
										#else
											messageLength = sprintf(message, "t,%hu,\n", 	controllerData.clientID);
											HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
										#endif
									continue;
								}
							}

							controllerState.pressureCompensation = COMPENSATION_ON;
						}
					}
					break;
				}
				case 'x':{
					if (controllerState.soundIndicationState == SEARCH_INDICATION){
						if (command[1] == '?'){
							sscanf((char*)command, "x?%hu,\n", &controllerData.clientID);
							messageLength = sprintf(message, "x,%05d,%05d,\n", controllerData.clientID, controllerState.serverUID);
							HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);
						}
						else if (command[1] == 'c'){
							sscanf((char*)command, "xc,%hu,%hu,\n", &id, &channel);

							if (id == controllerState.serverUID){
								controllerData.rfChannel = channel;
								mWrite_flash();
								messageLength = sprintf(message, "xc,%05d,ok,\n", controllerData.clientID);
								HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);

								vTaskDelay(200 / portTICK_RATE_MS);
								CMD_RF_ON;
								vTaskDelay(50 / portTICK_RATE_MS);

								messageLength = sprintf(message, "AT+C%03d\r", channel);
								HAL_UART_Transmit_DMA(&huart1, (uint8_t*) message, messageLength);

								vTaskDelay(50 / portTICK_RATE_MS);
								CMD_RF_OFF;

								controllerState.soundIndicationState = NORMAL_INDICATION;
							}
						}
					}
					break;
				}
				default:{
					break;
				}
			}
		}
	}
	vTaskDelete(NULL);
}
