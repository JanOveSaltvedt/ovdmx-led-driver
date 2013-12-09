/*
 * com.h
 *
 * Created: 8/10/2013 11:48:19 AM
 *  Author: Jan Ove Saltvedt
 */ 


#ifndef COM_H_
#define COM_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"

int com_init();

void com_send(char* msg);
void com_sendf(const char *fmt, ...);


#endif /* COM_H_ */