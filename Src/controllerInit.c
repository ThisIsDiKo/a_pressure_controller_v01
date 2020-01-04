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

extern ADC_HandleTypeDef hadc1;
extern UART_HandleTypeDef huart1;

extern xSemaphoreHandle xPressureCompensationSemaphore;
extern xQueueHandle xRecCommandQueue;

extern char message[128];
extern uint8_t recCommandByte;


void xBlynkTask(void* arguments){
	for(;;){
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

	mRead_flash();
	if(controllerData.rfChannel > 120){
		controllerData.rfChannel = 1;
	}

	HAL_GetUID(unique_ID);
	controllerState.serverUID = unique_ID[0] + unique_ID[1] + unique_ID[2];

	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);


	init_rf433(controllerData.rfChannel); //TODO: change to more common



	vSemaphoreCreateBinary(xPressureCompensationSemaphore);

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
				512,
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

	HAL_UART_Receive_IT(&huart1, &recCommandByte, 1);
	HAL_ADCEx_InjectedStart_IT(&hadc1);
}
