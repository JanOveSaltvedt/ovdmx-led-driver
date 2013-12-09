/*
 * watchdog.h
 *
 * Created: 01.12.2013 12:27:21
 *  Author: janov_000
 */ 


#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include <avr/io.h>

void watchdog_init(void);
void watchdog_ping(void);



#endif /* WATCHDOG_H_ */