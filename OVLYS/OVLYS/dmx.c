/*
 * dmx.c
 *
 * Created: 26.10.2013 18:30:04
 *  Author: janov_000
 */ 

#include "dmx.h"
#include "config.h"
#include "timer.h"
#include "com.h"

#define DMX_STATE_IDLE		0
#define DMX_STATE_IN_BREAK	1
#define DMX_STATE_IN_MAB	2
#define DMX_STATE_DATA		3

#define DMX_USART			USARTC1
#define DMX_USART_PORT		PORTC
#define DMX_USART_PIN_TX	PIN7_bm
#define DMX_USART_PIN_RX	PIN6_bm
#define DMX_USART_RX_PINCTRL PIN6CTRL
#define DMX_USART_RX_PININT_VECTOR PORTC_INT0_vect
#define DMX_USART_RXC_INT	USARTC1_RXC_vect
#define DMX_RE_PORT			PORTB
#define DMX_RE_PIN			PIN2_bm
#define DMX_DE_PORT			PORTB
#define DMX_DE_PIN			PIN3_bm

uint8_t dmx_state = DMX_STATE_IDLE;
volatile timer_t dmx_timer_break;

volatile dmx_packet_t dmx_buffers[3];
volatile dmx_packet_t* dmx_front;
volatile uint8_t dmx_new_middle = 0;
volatile dmx_packet_t* dmx_middle;
volatile dmx_packet_t* dmx_back;

void dmx_enable_receiver(void);
void dmx_enable_interrupt(void);
void dmx_disable_receiver(void);
void dmx_disable_interrupt(void);

void dmx_init(void) {
	// Set up USART for DMX. 250 kbaud, 1 start bit, 8 data bits, 2 stop bits
	// Set TX to output and RX to input
	DMX_USART_PORT.DIRSET = DMX_USART_PIN_TX;
	DMX_USART_PORT.DIRCLR = DMX_USART_PIN_RX;
	
	DMX_USART.CTRLC = USART_CHSIZE_8BIT_gc | USART_SBMODE_bm;
	
	// Set baudrate to 250kbaud (at 32MHz)
	DMX_USART.BAUDCTRLA = 1;
	DMX_USART.BAUDCTRLB = (2 << USART_BSCALE0_bp);
	
	// ENABLE interrupts
	DMX_USART.CTRLA |= USART_RXCINTLVL_LO_gc;
	DMX_USART.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
	
	// Set the rs485 enable pins to output
	DMX_RE_PORT.DIRSET = DMX_RE_PIN;
	DMX_DE_PORT.DIRSET = DMX_DE_PIN;
	
	// Set the rs485 chip to receiving mode
	DMX_RE_PORT.OUTCLR = DMX_RE_PIN;
	DMX_DE_PORT.OUTCLR = DMX_DE_PIN;
	
	// Set up the buffers
	for (uint8_t i = 0; i < 3; i++)
	{
		dmx_packet_t* packet = &dmx_buffers[i];
		packet->start_code = 0;
		packet->data_length = 0;
		for (uint16_t a = 0; a < 512; a++)
		{
			packet->data[i] = 0;
		}
	}
	
	// Place buffers
	dmx_front = &dmx_buffers[0];
	dmx_middle = &dmx_buffers[1];
	dmx_back = &dmx_buffers[2];	
}

uint32_t ntohl(uint32_t input) {
	return ((input>>24)&0xff) | // move byte 3 to byte 0
	((input<<8)&0xff0000) | // move byte 1 to byte 2
	((input>>8)&0xff00) | // move byte 2 to byte 1
	((input<<24)&0xff000000); // byte 0 to byte 3
}

uint16_t ntohs(uint16_t input) {
	return (input << 8) | (input >> 8);
}

void dmx_put(uint8_t data) {
	while((DMX_USART.STATUS & USART_DREIF_bm) == 0);
	DMX_USART.DATA = data;
}
		
void dmx_transfer(uint8_t* data, uint16_t len) {
	for(uint16_t i = 0; i < len; i++) {
		dmx_put(data[i]);
	}
	while((DMX_USART.STATUS & USART_DREIF_bm) == 0);
	// Wait for transfer to complete
}

void dmx_on_rdm_discovery(rdm_frame_header_t* frame) {
	static uint8_t is_muted = 0;
	
	if(frame->parameter_id == RDM_PID_DISC_MUTE) {
		is_muted = 1;
		com_send("Muted...\n");
	}
	else if(frame->parameter_id == RDM_PID_DISC_UN_MUTE) {
		is_muted = 0;
		com_send("Un muted...\n");
	}
	else if(frame->parameter_id == RDM_PID_DISC_UNIQUE_BRANCH) {
		// Decode the data
		frame->disc_unique_branch_data.lower_bound_mid = ntohs(frame->disc_unique_branch_data.lower_bound_mid);
		frame->disc_unique_branch_data.lower_bound_did = ntohl(frame->disc_unique_branch_data.lower_bound_did);
		
		frame->disc_unique_branch_data.upper_bound_mid = ntohs(frame->disc_unique_branch_data.upper_bound_mid);
		frame->disc_unique_branch_data.upper_bound_did = ntohl(frame->disc_unique_branch_data.upper_bound_did);
		
		// Are we included in the range?
		config_t* config = config_get();
		//com_sendf("S: 0x%X, 0x%lX, U: 0x%X, 0x%lX\n", frame->disc_unique_branch_data.lower_bound_mid, frame->disc_unique_branch_data.lower_bound_did, frame->disc_unique_branch_data.upper_bound_mid, frame->disc_unique_branch_data.upper_bound_did);
		if(!(config->manufacter_id >= frame->disc_unique_branch_data.lower_bound_mid && config->manufacter_id <= frame->disc_unique_branch_data.upper_bound_mid)) {
			// We are not interested
			return;
		}
		
		if (frame->disc_unique_branch_data.lower_bound_did != RDM_BROADCAST_ALL_DID || frame->disc_unique_branch_data.upper_bound_did != RDM_BROADCAST_ALL_DID)
		{
			// We are sorting on device ID
			if(!(config->device_id >= frame->disc_unique_branch_data.lower_bound_did && config->device_id <= frame->disc_unique_branch_data.upper_bound_did)) {
				// We are not interested
				return;
			}
		}
		
		if(is_muted) {
			// We are not allowed to respond to this message.
			return;
		}
		
		
		
		uint8_t data[24];
		// Response preamble
		data[0] = 0xFE;
		data[1] = 0xFE;
		data[2] = 0xFE;
		data[3] = 0xFE;
		data[4] = 0xFE;
		data[5] = 0xFE;
		data[6] = 0xFE;
		// Response preamble separator
		data[7] = 0xAA;
		
		// MID
		// MSB
		data[8] = (config->manufacter_id>>8)|0xAA;
		data[9] = (config->manufacter_id>>8)|0x55;
		// LSB
		data[10] = (config->manufacter_id)|0xAA;
		data[11] = (config->manufacter_id)|0x55;
		
		// DID
		// MSB ID3
		data[12] = (config->device_id>>24)|0xAA;
		data[13] = (config->device_id>>24)|0x55;
		// ID2
		data[14] = (config->device_id>>16)|0xAA;
		data[15] = (config->device_id>>16)|0x55;
		// ID1
		data[16] = (config->device_id>>8)|0xAA;
		data[17] = (config->device_id>>8)|0x55;
		// ID0
		data[18] = (config->device_id)|0xAA;
		data[19] = (config->device_id)|0x55;
		
		// Checsum
		uint16_t crc = 0;
		for (uint8_t i = 0; i < 20; i++)
		{
			crc += data[i];
		}
		// MSB
		data[20] = (crc>>8)|0xAA;
		data[21] = (crc>>8)|0x55;
		// LSB
		data[22] = (crc)|0xAA;
		data[23] = (crc)|0x55;
		
		// Set the rs485 chip to transmitting mode
		DMX_RE_PORT.OUTSET = DMX_RE_PIN;
		DMX_DE_PORT.OUTSET = DMX_DE_PIN;
		
		dmx_transfer(data, 24);
		
		// Set the rs485 chip to receiving mode
		DMX_RE_PORT.OUTCLR = DMX_RE_PIN;
		DMX_DE_PORT.OUTCLR = DMX_DE_PIN;
		
		com_send("Sent data...\n");
	}
}

void dmx_on_rdm_frame(dmx_packet_t* frame) {
	
	config_t* config = config_get();
	
	rdm_frame_header_t* header = (rdm_frame_header_t*)((void*)frame->data);
	// AVR is using little endian, and DMX big-endian. So we have to switch endian
	header->destination_mid = ntohs(header->destination_mid);
	header->destination_did = ntohl(header->destination_did);
	header->sub_device = ntohs(header->sub_device);
	header->parameter_id = ntohs(header->parameter_id);
	
	// Check if we are interested in this frame
	if(!(header->destination_mid == RDM_BROADCAST_ALL_MID  || header->destination_mid == config->manufacter_id)) {
		// The message was not intended for us
		return;
	}
	
	if (!(header->destination_did == RDM_BROADCAST_ALL_DID || header->destination_did == config->device_id))
	{
		// The message was not intended for us
		return;
	}
	
	// Great. Now we must check if the message is valid by doing a CRC on it.
	uint16_t crc = frame->start_code;
	for(uint8_t i = 0; i < header->message_length-1; i++) {
		crc += frame->data[i];
	}			
	
	uint16_t correct_crc = ntohs(*(uint16_t*)(&frame->data[header->message_length-1]));
	if(crc != correct_crc) {
		// CRC did not match...
		return;
	}
	
	// The message is valid.
	// Now lets do shit...
	if(header->command_class == RDM_CMD_DISCOVERY) {
		
		dmx_on_rdm_discovery(header);
	}
}

void dmx_on_frame_end(void) {
	// Fill out the rest of the packet with zeros
	for(uint16_t i = dmx_back->data_length; i < 512; i++) {
		dmx_back->data[i] = 0;
	}
	dmx_back->data_length = 512;
	
	if(dmx_back->start_code == 0x00) {
		// We are done with the packet, swap back and middle buffers
		dmx_packet_t* new_back = dmx_middle;
		dmx_middle = dmx_back;
		dmx_back = new_back;
		
		// Set new back buffer to empty
		dmx_back->data_length = 0;
		
		dmx_new_middle = 1;
	}
	else if(dmx_back->start_code == 0xCC && dmx_back->data[0] == 0x01) {
		// This was a RDM frame
		// Handle the message
		dmx_on_rdm_frame(dmx_back);
		// Set back buffer to empty
		dmx_back->data_length = 0;
	}
	else 
	{
		// Invalid frame... Ignore it like nothing happened.
		// Set back buffer to empty
		dmx_back->data_length = 0;
	}
		
	dmx_state = DMX_STATE_IDLE;
	
}

// Interrupt for usart receive
ISR(DMX_USART_RXC_INT) {
	
	// This should not really be needed. But oh well
	if(DMX_USART.STATUS & USART_FERR_bm) {
		// Frame error means break
		dmx_state = DMX_STATE_IN_MAB;
		// Read the data register so we jump to the next frame...
		uint8_t b = DMX_USART.DATA;
		
		// If we are working with a packet at the moment, then it also means we are done with it
		if(dmx_back->data_length > 0) {
			dmx_on_frame_end();
		}

		return;
	}
	
	uint8_t b = DMX_USART.DATA;
	if(dmx_state == DMX_STATE_IN_MAB) {
		// We got the start byte
		if(b==0x00 || b==0xCC) { // Valid start byte
			// Regular DMX frame or RDM frame
			dmx_back->start_code = b;
			dmx_back->data_length = 0;
			dmx_state = DMX_STATE_DATA;
		}
		else {
			dmx_state = DMX_STATE_IDLE;
		}
	}
	else if(dmx_state == DMX_STATE_DATA) {
		dmx_back->data[dmx_back->data_length] = b;
		dmx_back->data_length++;
		
		if(dmx_back->data_length >= 512) {
			
			dmx_on_frame_end();
		}
	}
}

uint8_t dmx_has_new_data(void) {
	return dmx_new_middle;
}

dmx_packet_t* dmx_get_active_packet(void) {
	if(dmx_new_middle == 0) {
		return dmx_front;
	}
	// We have a new middle swap buffer
	dmx_new_middle = 0;
	// Swap front and middle buffers
	dmx_packet_t* new_front = dmx_middle;
	dmx_middle = dmx_front;
	dmx_front = new_front;
	return new_front;
}
 uint8_t dmx_get_state(void) {
	 return dmx_state;
 }