/*
 * com.c
 *
 * Created: 8/10/2013 11:48:48 AM
 *  Author: Jan Ove Saltvedt
 */ 

#include <stdio.h>
#include <string.h>

#include "com.h"
#include "dmx.h"
#include "config.h"
#include "device.h"

#include <stddef.h>

#define COM_USART	&USARTD1

int com_init() {
	// Init usart C1 (Called UC2_USART ON BOARD)
	PORTD.DIRSET = PIN7_bm;
	PORTD.DIRCLR = PIN6_bm;
	
	usart_enable_rx_int(COM_USART);
	// 115200 bps at 32 Mhz
	usart_init(COM_USART, 2094, 0b1001);
	return 1;
}

void com_send(char* msg) {
	usart_putstring(COM_USART, msg);
}

void com_handle_message(char* msg) {
	static char outputBuffer[128];
	
	if(strcmp("show_packet", msg) == 0){
		dmx_packet_t* packet = dmx_get_active_packet();
		sprintf(outputBuffer,"SC: 0x%X LEN: %d DATA: ", packet->start_code, packet->data_length);
		com_send(outputBuffer);
		for(uint16_t i = 0; i < packet->data_length; i++) {
			sprintf(outputBuffer,"%X", packet->data[i]);
			com_send(outputBuffer);
		}
		com_send("\n");
	}
	else if(strcmp("show_config", msg) == 0){
		config_t* config = config_get();
		sprintf(outputBuffer,"MID: 0x%X DID: 0x%lX CHANNELMAPPING:\n", config->manufacter_id, config->device_id);
		com_send(outputBuffer);
		for(uint8_t i = 0; i < CONFIG_NUM_CHANNELS; i++) {
			sprintf(outputBuffer,"  C[%d]=%d\n", i, config->channel_mapping[i]);
			com_send(outputBuffer);
		}
	}
	else if(strcmp("show_lot", msg) == 0){
		sprintf(outputBuffer,"LOT0: 0x%hhX\n", device_read_calibration_byte( offsetof( NVM_PROD_SIGNATURES_t,  LOTNUM0) ));
		com_send(outputBuffer);
	}
	
	else {
		com_send("Commands:\nshow_packet\nshow_config\n");
	}
}

void com_sendf(const char *fmt, ...) {
	static char buffer[128];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, 128, fmt, args);
	com_send(buffer);
	va_end(args);	
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