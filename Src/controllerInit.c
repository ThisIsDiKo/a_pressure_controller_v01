/*
 * controllerInit.c
 *
 *  Created on: 16 сент. 2019 г.
 *      Author: ADiKo
 */


#include "globals.h"

#include "controllerInit.h"

#include "flashFunctions.h"
#include "adcStorage.h"
#include "processCommand.h"
#include "analyze.h"
#include "structures.h"
#include "scanInput.h"
#include "controllerUtils.h"

#include "eeprom_driver.h"

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

extern xSemaphoreHandle xPressureCompensationSemaphore;
extern xSemaphoreHandle xSpiTxCompleteSemaphore;
extern xQueueHandle xRecCommandQueue;

extern char message[128];
extern uint8_t messageLength;
extern uint8_t recCommandByte;


void xBlynkTask(void* arguments){
	for(;;){
		HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);
		switch(controllerState.soundIndicationState){
		case SEARCH_INDICATION:
			HAL_GPIO_TogglePin(BUZZER_PORT, BUZZER_PIN);
			break;
		case NORMAL_INDICATION:
			HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
			break;
		default:
			break;
		}
		vTaskDelay(500 / portTICK_RATE_MS);
	}

	vTaskDelete(NULL);
}

void controller_init(){
	uint32_t unique_ID[3] = {0};

	init_structures();
	//eeprom_clear_pages();

	  eeprom_read_controller_info((uint8_t*)&controllerInfo);
	  if (controllerInfo.version != FIRMWARE_VERSION){
		  controllerInfo.version = FIRMWARE_VERSION;
		  eeprom_write_page(EEPROM_VERSION_ADDR, (uint8_t*) &controllerInfo.version, sizeof(controllerInfo.version));
		  HAL_Delay(5);

		  controllerInfo.aligningNum = 0;
		  eeprom_write_page(EEPROM_ALIGNING_NUM_ADDR, (uint8_t*) &controllerInfo.aligningNum, sizeof(controllerInfo.aligningNum));
		  HAL_Delay(5);

		  controllerInfo.connectionNum = 0;
		  eeprom_write_page(EEPROM_CONNECTION_NUM_ADDR, (uint8_t*) &controllerInfo.connectionNum, sizeof(controllerInfo.connectionNum));
		  HAL_Delay(5);

		  controllerInfo.errorAligningNum = 0;
		  eeprom_write_page(EEPROM_ERROR_ALIGNING_NUM_ADDR, (uint8_t*) &controllerInfo.errorAligningNum, sizeof(controllerInfo.errorAligningNum));
		  HAL_Delay(5);

		  controllerInfo.settingsAddress = 32;
		  eeprom_write_page(EEPROM_SETTINGS_ADDR, (uint8_t*) &controllerInfo.settingsAddress, sizeof(controllerInfo.settingsAddress));
		  HAL_Delay(5);

		  controllerInfo.turnOnNum = 0;
		  eeprom_write_page(EEPROM_TURN_ON_NUM_ADDR, (uint8_t*) &controllerInfo.turnOnNum, sizeof(controllerInfo.turnOnNum));
	  }



//	mRead_flash();
	  HAL_Delay(5);
	  eeprom_read_controller_settings(controllerInfo.settingsAddress, (uint8_t*) &controllerData);


	  if(controllerData.rfChannel > 120 || controllerData.rfChannel == 0){
		controllerData.rfChannel = 1;
		controllerData.writeCounter = 0;
		controllerData.clientID = 33333;
		controllerData.impDownCoeff[0] = 0.0;
		controllerData.impDownCoeff[1] = 0.0;
		controllerData.impDownCoeff[2] = 0.0;
		controllerData.impDownCoeff[3] = 0.0;

		controllerData.impUpCoeff[0] = 0.0;
		controllerData.impUpCoeff[1] = 0.0;
		controllerData.impUpCoeff[2] = 0.0;
		controllerData.impUpCoeff[3] = 0.0;

		controllerData.offsetPressure[0] = 0;
		controllerData.offsetPressure[1] = 0;
		controllerData.offsetPressure[2] = 0;
		controllerData.offsetPressure[3] = 0;

		controllerData.writeCounter += 1;
		eeprom_write_page(controllerInfo.settingsAddress, (uint8_t*)&controllerData, 16);
		HAL_Delay(8);

		eeprom_write_page(controllerInfo.settingsAddress+16, ((uint8_t*)(&controllerData))+16, 16);
		HAL_Delay(8);

		eeprom_write_page(controllerInfo.settingsAddress+32, ((uint8_t*)(&controllerData))+32, 16);
		HAL_Delay(8);
	}
	HAL_Delay(5);

	if(controllerData.rfChannel > 120){
		controllerData.rfChannel = 1;
	}

	HAL_GetUID(unique_ID);
	controllerState.serverUID = unique_ID[0] + unique_ID[1] + unique_ID[2];

	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);


	init_rf433(controllerData.rfChannel); //TODO: change to more common

#if DEBUG_EEPROM
	  eeprom_read_controller_info((uint8_t*)&controllerInfo);
	  messageLength = sprintf(message, "\n-------\n version: %d addr: %d turnon: %lu\n", controllerInfo.version, controllerInfo.settingsAddress, controllerInfo.turnOnNum);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "con: %d align: %lu error: %lu\n", controllerInfo.connectionNum, controllerInfo.aligningNum, controllerInfo.errorAligningNum);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "length: %d\n", sizeof(controllerData));
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
#endif

#if DEBUG_EEPROM
	  messageLength = sprintf(message, "ch: %d re: %d client: %d\n", controllerData.rfChannel, controllerData.rere, controllerData.clientID);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "U 1: %d 2: %d 3: %d 4: %d\n",(int8_t)controllerData.impUpCoeff[0], (int8_t)controllerData.impUpCoeff[1],(int8_t)controllerData.impUpCoeff[2],(int8_t)controllerData.impUpCoeff[3]);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "D 1: %d 2: %d 3: %d 4: %d\n",(int8_t)controllerData.impDownCoeff[0], (int8_t)controllerData.impDownCoeff[1],(int8_t)controllerData.impDownCoeff[2],(int8_t)controllerData.impDownCoeff[3]);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "Off 1: %d 2: %d 3: %d 4: %d\n",controllerData.offsetPressure[0], controllerData.offsetPressure[1],controllerData.offsetPressure[2],controllerData.offsetPressure[3]);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
	  messageLength = sprintf(message, "counter: %lu\n",controllerData.writeCounter);
	  HAL_UART_Transmit(&huart1, (uint8_t*) message, messageLength, 0x2000);
#endif


	vSemaphoreCreateBinary(xPressureCompensationSemaphore);
	vSemaphoreCreateBinary(xSpiTxCompleteSemaphore);

	#if DEBUG_SERIAL
		sprintf(message, "\n Server ID: %hu\r\n", controllerState.serverUID);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
		uint32_t fre=xPortGetFreeHeapSize();
		sprintf(message, "Free heap: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

	xTaskCreate(xBlynkTask,
				"Blynk",
				200,
				NULL,
				1,
				NULL);

	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after Blynk: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

	xTaskCreate(xScanInputTask,
				"Scan",
				200,
				NULL,
				1,
				NULL);

	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after Scan: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

	xTaskCreate(xStoreADCDataTask,
				"SADCData",
				512,
				NULL,
				1,
				NULL);

	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after SADCData: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

	xTaskCreate(xProcessCommandTask,
				"ProcCmd",
				700,
				NULL,
				3,
				NULL);

	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after ProcCmd: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

	xTaskCreate(xAnalyzeTask,
				"AnTsk",
				800,
				NULL,
				1,
				NULL);
	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after AnTask: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*) message, strlen(message), 0xFFFF);
	#endif

	xRecCommandQueue = xQueueCreate(COMMAND_QUEUE_SIZE, MAX_COMMAND_LENGTH);

	#if DEBUG_SERIAL
		fre=xPortGetFreeHeapSize();
		sprintf(message, "heap after queue: %ld\r\n", fre);
		HAL_UART_Transmit(&huart1, (uint8_t*)message, strlen(message), 0xFFFF);
	#endif

		  controllerInfo.turnOnNum++;
		  eeprom_wren();
		  eeprom_write_page(EEPROM_TURN_ON_NUM_ADDR, (uint8_t*) &controllerInfo.turnOnNum, sizeof(controllerInfo.turnOnNum));
		  //while(xSemaphoreTake( xSpiTxCompleteSemaphore, portMAX_DELAY ) != pdTRUE);

	HAL_UART_Receive_IT(&huart1, &recCommandByte, 1);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
}
