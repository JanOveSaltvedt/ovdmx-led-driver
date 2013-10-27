/*
 * timer.h
 *
 * Created: 27.10.2013 13:05:30
 *  Author: janov_000
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "TC_driver.h"

#define MAX_TIMERS 8

typedef struct {
	uint16_t start_value;
	uint16_t iteration_count;
	uint16_t stop_value;
} timer_t;

/************************************************************************
This timer reuses the PWM timers. They are set to 4Mhz, if not you must
change the constants below of this library.                                                                    
************************************************************************/
#define TIMER_PERIOD	4096UL
#define TIMER_TIMER		TCC0
#define TIMER_TIMER_INT TCC0_OVF_vect
void timer_init(void);
void timer_start(timer_t* timer);
void timer_stop(timer_t* timer);

// Gets the number of ticks from the time timer_start() was called until timer_stop() was called
uint32_t timer_get_ticks(timer_t* timer);


#endif /* TIMER_H_ */