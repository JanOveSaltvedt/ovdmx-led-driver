/*
 * device.c
 *
 * Created: 31.10.2013 19:00:04
 *  Author: janov_000
 */ 


#include "device.h"
#include "crc.h"
#include <stddef.h>
#include <avr/pgmspace.h>

uint16_t device_manufacter_id = 1337;
uint32_t device_device_id = 0;

uint8_t ReadCalibrationByte( uint8_t index )
{
	uint8_t result;

	/* Load the NVM Command register to read the calibration row. */
	NVM_CMD = NVM_CMD_READ_CALIB_ROW_gc;
	result = pgm_read_byte(index);

	/* Clean up NVM Command register. */
	NVM_CMD = NVM_CMD_NO_OPERATION_gc;

	return( result );
}

uint8_t device_read_calibration_byte(uint8_t index ){
	return ReadCalibrationByte(index);
}

void device_init(void) {
	/* XMega serial number: */
	crc_init();
	(void) ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, LOTNUM0 ) ); /* First read after reset or possibly Power Up returns zero, so read then toss this value */
	
	crc_reset();
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM0) ));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM1) ));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM2) ));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM3) ));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM4) ));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM5) ));
	
	uint16_t high_word = crc_get();
	crc_reset();
		
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, WAFNUM )));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, COORDX0 )));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, COORDX1 )));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, COORDY0 )));
	crc_add(ReadCalibrationByte( offsetof( NVM_PROD_SIGNATURES_t, COORDY1 )));
	
	uint16_t low_word = crc_get();
	device_device_id = (((uint32_t)high_word) << 16) | ((uint32_t)low_word);
}

uint16_t device_get_manufacter_id(void) {
	return device_manufacter_id;
}

uint32_t device_get_device_id(void) {
	return device_device_id;
}