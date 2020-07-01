/*
 * eepromTasks.c
 *
 *  Created on: 26 θών. 2020 γ.
 *      Author: ADiKo
 */

#include "globals.h"
#include "structures.h"
#include "eepromTasks.h"
#include "eeprom_driver.h"

extern UART_HandleTypeDef huart1;

void xEepromWriteSettings(void* arguments){
	for(;;){
		#if DEBUG_SERIAL
			HAL_UART_Transmit(&huart1, (uint8_t*) "EEPROM\n", 7, 0xFFFF);
		#endif
		eeprom_write_page(controllerInfo.settingsAddress, (uint8_t*)&controllerData, 16);
		//while(xSemaphoreTake( xSemaphore, LONG_TIME ) != pdTRUE);
		vTaskDelay(8);
		eeprom_write_page(controllerInfo.settingsAddress+16, (uint8_t*)(&controllerData)+16, 16);
		vTaskDelay(8);
		eeprom_write_page(controllerInfo.settingsAddress+32, (uint8_t*)(&controllerData)+32, 16);
		vTaskDelay(8);
		vTaskDelete(NULL);
	}
	vTaskDelete(NULL);
}
