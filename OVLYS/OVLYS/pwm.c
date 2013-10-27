/*
 * pwm.c
 *
 * Created: 8/10/2013 3:11:42 PM
 *  Author: Jan Ove Saltvedt
 */ 

#include "pwm.h"
#include "TC_driver.h"

#define DEFAULT_VALUE 0	
#define MIN_VALUE 0
#define MAX_VALUE 4095

register16_t* pwm_outputs[16] = {&TCE0.CCA, &TCE0.CCB, &TCE0.CCC, &TCD0.CCD, &TCD1.CCA, &TCD1.CCB, &TCD0.CCA, &TCD0.CCB, &TCD0.CCC, &TCC0.CCD, &TCC1.CCA, &TCC1.CCB, &TCC0.CCA, &TCC0.CCB, &TCC0.CCC, &TCE0.CCD};
uint16_t led_correction[] = LED_CORRECTION;


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
	
	// Start timers
	TC0_ConfigClockSource(&TCC0, TC_CLKSEL_DIV8_gc);
	TC0_ConfigClockSource(&TCC1, TC_CLKSEL_DIV8_gc);
	TC0_ConfigClockSource(&TCD0, TC_CLKSEL_DIV8_gc);
	TC0_ConfigClockSource(&TCD1, TC_CLKSEL_DIV8_gc);
	TC0_ConfigClockSource(&TCE0, TC_CLKSEL_DIV8_gc);
	
	return 0;
}

void pwm_set_value(uint8_t n, uint16_t val) {
	*pwm_outputs[n] = val;
}

void pwm_set_led_value(uint8_t n, uint8_t val) {
	pwm_set_value(n, led_correction[val]);
}