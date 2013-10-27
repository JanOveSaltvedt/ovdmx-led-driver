/*
 * dmx.h
 *
 * Created: 26.10.2013 18:29:24
 *  Author: janov_000
 */ 


#ifndef DMX_H_
#define DMX_H_

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct {
	uint8_t start_code;
	uint16_t data_length;
	uint8_t data[512];
} dmx_packet_t;

void dmx_init(void);
dmx_packet_t* dmx_get_active_packet(void);
uint8_t dmx_get_state(void);

#endif /* DMX_H_ */