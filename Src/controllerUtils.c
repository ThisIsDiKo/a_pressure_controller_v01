/*
 * controllerUtils.c
 *
 *  Created on: 16 сент. 2019 г.
 *      Author: ADiKo
 */

#include "structures.h"

extern UART_HandleTypeDef huart1;

char debugMessage[16] = {};
uint8_t debugMessageLength = 0;

void print_debug(char* msg){
	#if DEBUG_SERIAL
		debugMessageLength = sprintf(debugMessage, "%s", msg);
		HAL_UART_Transmit(&huart1, (uint8_t*) debugMessage, debugMessageLength, 0x2000);
	#endif
}

//void init_rf433(uint8_t channel){
//	CMD_RF_ON;
//	HAL_Delay(50);
//
//	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+FU1\r", 7, 0x2000);
//	HAL_Delay(200);
//	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+B19200\r", 7, 0x2000);
//	HAL_Delay(200);
//	debugMessageLength = sprintf(debugMessage, "AT+C%03d\r", channel);
//	HAL_UART_Transmit(&huart1, (uint8_t*) debugMessage, debugMessageLength, 0x2000);
//	HAL_Delay(200);
//
//	CMD_RF_OFF;
//	HAL_Delay(50);
//
//	huart1.Init.BaudRate = 19200;
//	if (HAL_UART_Init(&huart1) != HAL_OK){
//		//Error_Handler();
//	}
//}

void init_rf433(uint8_t channel){
	char bufLenRf = 0;
	char bufRf[10] = {0};
	uint8_t recVal = 0;

	CMD_RF_ON;
	HAL_Delay(50);

	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+FU1\r", 7, 0x2000);
	HAL_UART_Receive(&huart1, &recVal, 1, 100);
	HAL_Delay(200);

	if (recVal != 'O'){
		huart1.Init.BaudRate = 19200;
		if (HAL_UART_Init(&huart1) != HAL_OK){

		}
		HAL_UART_Transmit(&huart1, (uint8_t*) "AT+FU1\r", 7, 0x2000);
		HAL_Delay(200);

	}

	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+P8\r", 6, 0x2000);
	HAL_Delay(200);

//	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+DEFAULT\r", 11, 0x2000);
//	HAL_Delay(200);

	HAL_UART_Transmit(&huart1, (uint8_t*) "AT+B19200\r", 10, 0x2000);
	HAL_Delay(200);
	if (channel == 0 || channel > 127) channel = 1;
	bufLenRf = sprintf(bufRf, "AT+C%03d\r", channel);
	HAL_UART_Transmit(&huart1, (uint8_t*) bufRf, bufLenRf, 0x2000);
	HAL_Delay(200);

	CMD_RF_OFF;
	HAL_Delay(50);

	huart1.Init.BaudRate = 19200;
	if (HAL_UART_Init(&huart1) != HAL_OK){

	}
}

void init_structures(){

	controllerState.analyzeState = COMPENSATION_STATE_FREE;
	controllerState.pressureCompensation = COMPENSATION_OFF;
	controllerState.soundIndicationState = NORMAL_INDICATION;
	controllerState.airSystemType = RECEIVER;
	controllerState.errorStatus = STATUS_NORMAL;

	controllerState.nessPressure[0] = 0;
	controllerState.nessPressure[1] = 0;
	controllerState.nessPressure[2] = 0;
	controllerState.nessPressure[3] = 0;

	controllerState.filteredData[0] = 0;
	controllerState.filteredData[1] = 0;
	controllerState.filteredData[2] = 0;
	controllerState.filteredData[3] = 0;

	controllerState.serverUID = 0;
	controllerState.errorByte = 0;
	controllerState.errorMeaningByte = 0;
	controllerState.waysType = 6;
	controllerState.analyzeAccuracy = 40;
	controllerState.status = 0;
	controllerState.numberOfTries = 0;
	controllerState.lastTimeCommand = 0;
	controllerState.compressorWorkTime = 0;
}
