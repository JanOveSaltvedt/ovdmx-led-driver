/*
 * pwm.c
 *
 * Created: 8/10/2013 3:11:42 PM
 *  Author: Jan Ove Saltvedt
 */ 

#include "pwm.h"
#include "TC_driver.h"
#include <avr/interrupt.h>

#define DEFAULT_VALUE 0	
#define MIN_VALUE 0
#define MAX_VALUE 4095

register16_t* pwm_outputs[16] = {
	&TCE0.CCCBUF, &TCE0.CCBBUF, &TCE0.CCABUF, // GROUP 1
	&TCD1.CCBBUF, &TCD1.CCABUF, &TCD0.CCDBUF, // GROUP 2
	&TCD0.CCCBUF, &TCD0.CCBBUF, &TCD0.CCABUF, // GROUP 3
	&TCC1.CCBBUF, &TCC1.CCABUF, &TCC0.CCDBUF, // GROUP 4
	&TCC0.CCCBUF, &TCC0.CCBBUF, &TCC0.CCABUF, // GROUP 5
	&TCE0.CCDBUF}; // Status
uint16_t led_correction[] = LED_CORRECTION;

volatile pwm_update_callback_t pwm_update_callback = 0;

uint8_t pwm_init(void) {	
	// Set all pwm pins to outputs
	PORTC.DIRSET = 0b00111111;
	PORTD.DIRSET = 0b00111111;
	PORTE.DIRSET = 0b00001111;
	
	// Set timers to 976 Hz pwm
	TC_SetPeriod(&TCC0, 4095);
	TC_SetPeriod(&TCC1, 4095);
	TC_SetPeriod(&TCD0, 4095);
	TC_SetPeriod(&TCD1, 4095);
	TC_SetPeriod(&TCE0, 4095);
	
	// Set to singleslope PWM
	TC0_ConfigWGM(&TCC0, TC_WGMODE_SS_gc);
	TC0_ConfigWGM(&TCC1, TC_WGMODE_SS_gc);
	TC0_ConfigWGM(&TCD0, TC_WGMODE_SS_gc);
	TC0_ConfigWGM(&TCD1, TC_WGMODE_SS_gc);
	TC0_ConfigWGM(&TCE0, TC_WGMODE_SS_gc);
	
	// Enable compare channels
	TC0_EnableCCChannels(&TCC0, TC0_CCAEN_bm);
	TC0_EnableCCChannels(&TCC0, TC0_CCBEN_bm);
	TC0_EnableCCChannels(&TCC0, TC0_CCCEN_bm);
	TC0_EnableCCChannels(&TCC0, TC0_CCDEN_bm);
	TC1_EnableCCChannels(&TCC1, TC1_CCAEN_bm);
	TC1_EnableCCChannels(&TCC1, TC1_CCBEN_bm);
	
	TC0_EnableCCChannels(&TCD0, TC0_CCAEN_bm);
	TC0_EnableCCChannels(&TCD0, TC0_CCBEN_bm);
	TC0_EnableCCChannels(&TCD0, TC0_CCCEN_bm);
	TC0_EnableCCChannels(&TCD0, TC0_CCDEN_bm);
	TC1_EnableCCChannels(&TCD1, TC1_CCAEN_bm);
	TC1_EnableCCChannels(&TCD1, TC1_CCBEN_bm);
	
	TC0_EnableCCChannels(&TCE0, TC0_CCAEN_bm);
	TC0_EnableCCChannels(&TCE0, TC0_CCBEN_bm);
	TC0_EnableCCChannels(&TCE0, TC0_CCCEN_bm);
	TC0_EnableCCChannels(&TCE0, TC0_CCDEN_bm);
	
	
	// Set default values
	for(int i = 0; i < 16; i++) {
		*pwm_outputs[i] = DEFAULT_VALUE; 
	}
	
	// Start timers with equal spacing between. (We avoid that all the power is drawn at the same time...)
	TC0_ConfigClockSource(&TCC0, TC_CLKSEL_DIV8_gc);
	while(TCC0.CNT <= (4096/5)*1);	
	TC1_ConfigClockSource(&TCC1, TC_CLKSEL_DIV8_gc);
	while(TCC0.CNT <= (4096/5)*2);
	TC0_ConfigClockSource(&TCD0, TC_CLKSEL_DIV8_gc);
	while(TCC0.CNT <= (4096/5)*3);
	TC1_ConfigClockSource(&TCD1, TC_CLKSEL_DIV8_gc);
	while(TCC0.CNT <= (4096/5)*4);
	TC0_ConfigClockSource(&TCE0, TC_CLKSEL_DIV8_gc);	
	return 0;
}

ISR(TCD0_OVF_vect) {
	if(pwm_update_callback != 0) {
		(*pwm_update_callback)();
	}
}

void pwm_set_on_update_callback(pwm_update_callback_t cb) {
	pwm_update_callback = cb;
	// Enable the interrupt
	cli();
	TC0_SetOverflowIntLevel(&TCD0, TC_OVFINTLVL_LO_gc);
	sei();
}

void pwm_set_value(uint8_t n, uint16_t val) {
	*pwm_outputs[n] = val;
}

void pwm_set_led_value(uint8_t n, uint8_t val) {
	pwm_set_value(n, led_correction[val]);
}