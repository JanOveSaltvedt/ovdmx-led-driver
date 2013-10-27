/*
 * timer.c
 *
 * Created: 27.10.2013 13:05:39
 *  Author: janov_000
 */ 

#include "timer.h"
#include "com.h"
volatile timer_t* active_timers[MAX_TIMERS];

void timer_init(void) {
	for(uint8_t i = 0; i < MAX_TIMERS; i++) {
		active_timers[i] = 0;
	}
	TIMER_TIMER.INTCTRLA |= TC_OVFINTLVL_LO_gc;		
}

ISR(TIMER_TIMER_INT) {
	for(uint8_t i = 0; i < MAX_TIMERS; i++) {
		if(active_timers[i] != 0) {
			active_timers[i]->iteration_count++;
		}
	}
}

void timer_start(timer_t* timer) {
	timer->start_value = TIMER_TIMER.CNT;
	timer->iteration_count = 0;
	// Make sure its not already running, if so stop it
	timer_stop(timer);
	
	// Place it in the next available spot
	for(uint8_t i = 0; i < MAX_TIMERS; i++) {
		if(active_timers[i] == 0) {
			active_timers[i] = timer;
			break;
		}
	}
}

void timer_stop(timer_t* timer) {
	timer->stop_value = TIMER_TIMER.CNT;
	// Remove all timers that points to timer
	for(uint8_t i = 0; i < MAX_TIMERS; i++) {
		if(active_timers[i] == timer) {
			active_timers[i] = 0;
		}
	}
}

uint32_t timer_get_ticks(timer_t* timer) {
	return ((uint32_t)timer->iteration_count)*TIMER_PERIOD + (uint32_t)timer->stop_value - (uint32_t)timer->start_value;
}