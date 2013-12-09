/*
 * watchdog.c
 *
 * Created: 01.12.2013 12:28:23
 *  Author: janov_000
 */ 

#include "watchdog.h"

void watchdog_init(void) {
	uint8_t temp = WDT.CTRL | WDT_ENABLE_bm | WDT_CEN_bm | WDT_PER_4KCLK_gc;
	CCP = CCP_IOREG_gc;
	WDT.CTRL = temp;
}

void watchdog_ping(void) {
	asm("wdr");
}