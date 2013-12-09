/*
 * crc.h
 *
 * Created: 29.10.2013 11:07:39
 *  Author: janov_000
 */ 


#ifndef CRC_H_
#define CRC_H_

#include <avr/io.h>

void crc_init(void);
void crc_reset(void);
void crc_add(uint8_t b);
uint16_t crc_get(void);



#endif /* CRC_H_ */