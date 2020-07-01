/*
 * eeprom_driver.h
 *
 *  Created on: 26 θών. 2020 γ.
 *      Author: ADiKo
 */

#ifndef EEPROM_DRIVER_H_
#define EEPROM_DRIVER_H_

uint8_t eeprom_read_byte(uint8_t address);
uint8_t eeprom_write_byte(uint8_t address, uint8_t data);
uint8_t eeprom_write_page(uint8_t address, uint8_t* data, uint8_t size);
uint8_t eeprom_write_page_isr(uint8_t address, uint8_t* data, uint8_t size);
uint8_t eeprom_read_page(uint8_t address, uint8_t* data, uint8_t size);
void eeprom_wren();

uint8_t eeprom_read_status();

void eeprom_read_controller_info(uint8_t* infoStruct);
void eeprom_write_controller_settings(uint8_t address, uint8_t* settings);
void eeprom_read_controller_settings(uint8_t address, uint8_t* settings);

void eeprom_clear_pages();

#endif /* EEPROM_DRIVER_H_ */
