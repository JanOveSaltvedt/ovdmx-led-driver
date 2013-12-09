/*
 * crc.c
 *
 * Created: 29.10.2013 11:08:43
 *  Author: janov_000
 */ 

#include "crc.h"

void crc_init(void) {	
	crc_reset();
}

void crc_reset(void) {
	// RESET TO ALL ONES
	CRC.CTRL = CRC_RESET_RESET1_gc;
	CRC.CTRL = CRC_SOURCE_IO_gc;
}

void crc_add(uint8_t b) {
	CRC.DATAIN = b;
}

uint16_t crc_get(void) {
	return CRC.CHECKSUM0 | ((uint16_t)CRC.CHECKSUM1<<8);
}