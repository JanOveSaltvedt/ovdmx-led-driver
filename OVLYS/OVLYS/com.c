/*
 * com.c
 *
 * Created: 8/10/2013 11:48:48 AM
 *  Author: Jan Ove Saltvedt
 */ 

#include "com.h"

#include <stdio.h>

#define COM_USART	&USARTD1

int com_init() {
	// Init usart C1 (Called UC2_USART ON BOARD)
	PORTD.DIRSET = PIN7_bm;
	PORTD.DIRCLR = PIN6_bm;
	
	usart_enable_rx_int(COM_USART);
	// 9600 bps at 32 Mhz
	usart_init(COM_USART, 207, 0);
	return 1;
}

void com_send(char* msg) {
	usart_putstring(COM_USART, msg);
}


void com_handle_message(char* msg) {
	com_send("Hello Message\n");
}

ISR(USARTD1_RXC_vect)
{
	static char read_buffer[128] = {0};
	static uint8_t read_buffer_index = 0;
	uint8_t c = usart_get(COM_USART);
	if(c == '\n' || c == '\r') {
		if(read_buffer_index == 0) 
			return;
		
		read_buffer[read_buffer_index] = '\0';
		com_handle_message(read_buffer);
		read_buffer_index = 0;
		return;		
	}
	
	read_buffer[read_buffer_index] = c;
	read_buffer_index++;
}