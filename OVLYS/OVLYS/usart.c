/*
 * usart.c
 *
 * Created: 8/10/2013 11:28:18 AM
 *  Author: Jan Ove Saltvedt
 */ 

#include "usart.h"

int usart_enable_rx_int(USART_t* usart) {
	usart->CTRLA |= USART_RXCINTLVL_LO_gc;
	return 1;
}

int usart_init(USART_t* usart, uint8_t baudSelectValue, uint8_t baudScaleFactor) {
	// Set format 8bit,no parity 1 stop bit
	usart->CTRLC = USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc;
	
	usart->BAUDCTRLA = (uint8_t)baudSelectValue;
	usart->BAUDCTRLB = (baudScaleFactor << USART_BSCALE0_bp) | (baudSelectValue>>8);
	
	// Enable tx and rx
	usart->CTRLB |= USART_TXEN_bm | USART_RXEN_bm;
	
	return 1;
}

void usart_set2x(USART_t* usart) {
	usart->CTRLB |= USART_CLK2X_bm;
}

void usart_put(USART_t* usart, uint8_t data) {
	while((usart->STATUS & USART_DREIF_bm) == 0);
	usart->DATA = data;
}

void usart_putstring(USART_t* usart, const char* string) {
	while(*string != '\0') {
		usart_put(usart, *string);
		string++;
	}
}

uint8_t usart_get(USART_t* usart) {
	while((usart->STATUS & USART_RXCIF_bm) == 0);
	return usart->DATA;
}