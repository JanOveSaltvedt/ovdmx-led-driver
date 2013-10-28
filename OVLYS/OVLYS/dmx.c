/*
 * dmx.c
 *
 * Created: 26.10.2013 18:30:04
 *  Author: janov_000
 */ 

#include "dmx.h"
#include "timer.h"

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
	DMX_USART.CTRLB |= USART_RXEN_bm;
	
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
			// Fill out the rest of the packet with zeros
			for(uint16_t i = dmx_back->data_length; i < 512; i++) {
				dmx_back->data[i] = 0;
			}
			dmx_back->data_length = 512;
			
			// We are done with the packet, swap back and middle buffers
			dmx_packet_t* new_back = dmx_middle;
			dmx_middle = dmx_back;
			dmx_back = new_back;
			
			dmx_new_middle = 1;
		}
		
		return;
	}
	
	uint8_t b = DMX_USART.DATA;
	if(dmx_state == DMX_STATE_IN_MAB) {
		// We got the start byte
		dmx_state = DMX_STATE_DATA;
		dmx_back->start_code = b;
		dmx_back->data_length = 0;
	}
	else if(dmx_state == DMX_STATE_DATA) {
		dmx_back->data[dmx_back->data_length] = b;
		dmx_back->data_length++;
		
		if(dmx_back->data_length >= 512) {
			
			dmx_packet_t* new_back = dmx_middle;
			dmx_middle = dmx_back;
			dmx_back = new_back;
			
			dmx_new_middle = 1;
			
			dmx_state = DMX_STATE_IDLE;
		}
	}
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