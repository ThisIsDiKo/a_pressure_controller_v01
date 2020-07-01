/*
 * eeprom_driver.c
 *
 *  Created on: 26 θών. 2020 γ.
 *      Author: ADiKo
 */
#include "globals.h"
#include "eeprom_driver.h"
#include "structures.h"

extern SPI_HandleTypeDef hspi1;

uint8_t eeprom_write_byte(uint8_t address, uint8_t data){
	uint8_t msg[3] = {0};

	msg[0] = 0x02;
	msg[1] = address;
	msg[2] = data;

	EEPROM_CS_ON;

	HAL_SPI_Transmit(&hspi1, msg, 3, 0x2000);

	EEPROM_CS_OFF;
	return 0;
}

uint8_t eeprom_read_byte(uint8_t address){
	uint8_t msg[2] = {0};
	msg[0] = 0x03;
	msg[1] = address;
	uint8_t recByte = 0;

	EEPROM_CS_ON;
	HAL_SPI_Transmit(&hspi1, msg, 2, 0x2000);
	HAL_SPI_Receive(&hspi1, &recByte, 1, 0x2000);

	EEPROM_CS_OFF;
	return recByte;

}

void eeprom_wren(){
	uint8_t wren = 0x06;
	EEPROM_CS_ON;
	HAL_SPI_Transmit(&hspi1, &wren, 1, 0x2000);
	EEPROM_CS_OFF;
}

uint8_t eeprom_read_status(){
	uint8_t rdsr = 0x05;
	uint8_t recByte = 128;

	EEPROM_CS_ON;

	HAL_SPI_Transmit(&hspi1, &rdsr, 1, 0x2000);
	HAL_SPI_Receive(&hspi1, &recByte, 1, 0x2000);

	EEPROM_CS_OFF;
	return recByte;
}

uint8_t eeprom_write_page_isr(uint8_t address, uint8_t* data, uint8_t size){
	eeprom_wren();
	if (size > 16){
		return 1;
	}
	uint8_t msg[18] = {0};
	uint8_t i = 0;
	msg[0] = 0x02;
	msg[1] = address;

	for (i = 0; i < size; i++){
		msg[i+2] = data[i];
	}

	EEPROM_CS_ON;
	//HAL_SPI_Transmit_IT(&hspi1, msg, size+2, 0x2000);
	HAL_SPI_Transmit_IT(&hspi1, msg, size+2);
	EEPROM_CS_OFF;

	return 0;
}

uint8_t eeprom_write_page(uint8_t address, uint8_t* data, uint8_t size){
	eeprom_wren();
	if (size > 16){
		return 1;
	}
	uint8_t msg[18] = {0};
	uint8_t i = 2;
	msg[0] = 0x02;
	msg[1] = address;


	for (i = 0; i < size; i++){
		msg[i+2] = data[i];
	}

//	while(size > 0){
//		msg[i] = (uint8_t)*data++;
//		i++;
//		size--;
//	}

	EEPROM_CS_ON;
	//HAL_SPI_Transmit_IT(&hspi1, msg, size+2, 0x2000);
	HAL_SPI_Transmit(&hspi1, msg, size+2, 0x2000);
	EEPROM_CS_OFF;

	return 0;
}

uint8_t eeprom_read_page(uint8_t address, uint8_t* data, uint8_t size){
	uint8_t msg[2] = {0};
	msg[0] = 0x03;
	msg[1] = address;

	EEPROM_CS_ON;
	HAL_SPI_Transmit(&hspi1, msg, 2, 0x2000);

	HAL_SPI_Receive(&hspi1, data, size, 0x2000);

	EEPROM_CS_OFF;
	return 0;
}

void eeprom_read_controller_info(uint8_t* infoStruct){
	eeprom_read_page(0, infoStruct, 16);
}


void eeprom_write_controller_settings(uint8_t address, uint8_t* settings){
	eeprom_write_page(address, settings, 16);
	HAL_Delay(5);
	eeprom_write_page(address+16, (uint8_t*)(settings+16), 16);
	HAL_Delay(5);
	eeprom_write_page(address+32, (uint8_t*)(settings+32), 16);
}

void eeprom_read_controller_settings(uint8_t address, uint8_t* settings){
	eeprom_read_page(address, settings, 48);
}

void eeprom_clear_pages(){
	uint8_t i = 0;
	uint8_t msg[18] = {0};
	msg[0] = 0x02;

	for (i = 0; i < 16; i++){
		eeprom_wren();
		msg[1] = i * 16;
		EEPROM_CS_ON;
		HAL_SPI_Transmit(&hspi1, msg, 18, 0x2000);
		EEPROM_CS_OFF;
		HAL_Delay(5);
	}


}

