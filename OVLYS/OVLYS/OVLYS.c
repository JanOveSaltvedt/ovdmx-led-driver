/*
 * OVLYS.c
 *
 * Created: 26.10.2013 14:49:54
 *  Author: janov_000
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#define START_ADDRESS 106 // 1 indexed
#define NUM_CHANNELS 15

#include "clksys_driver.h"
#include "pwm.h"
#include "com.h"
#include "timer.h"
#include "dmx.h"
#include "config.h"
#include "watchdog.h"

void on_pwm_update(void) {
	if(dmx_has_new_data()) {
		config_t* config = config_get();
		dmx_packet_t* packet = dmx_get_active_packet();
		for(uint8_t i = 0; i < CONFIG_NUM_CHANNELS; i++) {
			uint16_t c = config->channel_mapping[i];
			if(c < 0) {
				continue;
			}
			if(c >= packet->data_length) {
				continue;
			}
			pwm_set_led_value(i, packet->data[c]);
		}
	}
}


int main(void)
{
	// Set clock to 32 Mhz
	CLKSYS_Enable( OSC_RC32MEN_bm );
	CLKSYS_Prescalers_Config( CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc );
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	
	watchdog_init();
	com_init();
	config_init();
	pwm_init();
	
	timer_init();
	dmx_init();
	
	// Enable all interrupt levels
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	// Enable interrupts
	sei();
	
	// Set config
	//config_set_default();
	config_t* config = config_get();
	for (uint8_t i = 0; i < NUM_CHANNELS; i++)
	{
		config->channel_mapping[i] = START_ADDRESS+i-1;
	}
	config_save();
	
	com_send("Started up\n");
	
	//pwm_set_led_value(PWM_STATUS, 0);
	// Check if we have a special reset condition and show colors based on that
	if (RST.STATUS & RST_WDRF_bm)
	{
		// Watchdog reseted the device
		watchdog_ping();
		pwm_set_led_value(PWM_1, 64);
		RST.STATUS = RST_WDRF_bm;
		_delay_ms(2000);
		watchdog_ping();
	} 
	else if (RST.STATUS & RST_BORF_bm)
	{
		// Brownout reseted the device
		watchdog_ping();
		pwm_set_led_value(PWM_2, 64);
		RST.STATUS = RST_BORF_bm;
		_delay_ms(2000);
		watchdog_ping();
	}
	else {
		pwm_set_led_value(PWM_3, 64);
	}
	
	pwm_set_on_update_callback(&on_pwm_update);
    while(1)
    {
		watchdog_ping();
    }
}

