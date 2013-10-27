/*
 * usart.h
 *
 * Created: 8/10/2013 11:28:47 AM
 *  Author: Jan Ove Saltvedt
 */ 


#ifndef USART_H_
#define USART_H_

#include <avr/io.h>

int usart_enable_rx_int(USART_t* usart);

int usart_init(USART_t* usart, uint8_t baudSelectValue, uint8_t baudScaleFactor);

void usart_set2x(USART_t* usart);

void usart_put(USART_t* usart, uint8_t data);

void usart_putstring(USART_t* usart, const char* string);

uint8_t usart_get(USART_t* usart);

#endif /* USART_H_ */