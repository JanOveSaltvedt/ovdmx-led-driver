/*
 * config.c
 *
 * Created: 29.10.2013 11:12:58
 *  Author: janov_000
 */ 

#include <avr/eeprom.h>

#include "config.h"
#include "crc.h"
#include "device.h"

typedef struct {
	config_t config;
	uint16_t crc;
} eeprom_data_t;

eeprom_data_t EEMEM EEPROM_DATA;

eeprom_data_t eeprom_data_current;

void config_init(void) {
	crc_init();
	device_init();
	config_load();
}

uint16_t config_create_crc(void) {
	crc_reset();
	uint8_t* data = (uint8_t*)((void*)&eeprom_data_current.config);
	for(uint8_t i = 0; i < sizeof(config_t); i++) {
		crc_add(data[i]);
	}
	return crc_get();
}

void config_set_default(void) {
	eeprom_data_current.config.manufacter_id = device_get_manufacter_id();
	eeprom_data_current.config.device_id = device_get_device_id();
	for (uint8_t i = 0; i < CONFIG_NUM_CHANNELS; i++)
	{
		eeprom_data_current.config.channel_mapping[i] = -1;
	}
	eeprom_data_current.crc = config_create_crc();	
}

void config_load(void) {
	eeprom_read_block(&eeprom_data_current, &EEPROM_DATA, sizeof(eeprom_data_t));
	// Check crc
	uint16_t crc = config_create_crc();
	if(crc != eeprom_data_current.crc) {
		// Not a valid config, set to default values
		config_set_default();
		config_save();
		return;
	}
	return;
}

void config_save(void) {
	eeprom_data_current.crc = config_create_crc();	
	eeprom_write_block(&eeprom_data_current, &EEPROM_DATA, sizeof(eeprom_data_t));
}

config_t* config_get(void) {
	return &eeprom_data_current.config;
}