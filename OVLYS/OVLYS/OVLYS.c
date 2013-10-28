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

#include "clksys_driver.h"
#include "pwm.h"
#include "com.h"
#include "timer.h"
#include "dmx.h"

int main(void)
{
	// Set clock to 32 Mhz
	CLKSYS_Enable( OSC_RC32MEN_bm );
	CLKSYS_Prescalers_Config( CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc );
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	
	com_init();
	pwm_init();
	timer_init();
	dmx_init();
	
	// Enable all interrupt levels
	PMIC.CTRL |= PMIC_LOLVLEN_bm | PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm;
	// Enable interrupts
	sei();
	
	//com_send("Hello World\n");
	
    while(1)
    {
//         for (uint16_t i = 0; i < 256; i++)
//         {
// 			pwm_set_led_value(PWM_STATUS, 255-i);
// 			_delay_ms(10);
//         }
		dmx_packet_t* packet = dmx_get_active_packet();
		
		if(packet->data_length > 1) {
			pwm_set_led_value(PWM_STATUS, 255-packet->data[0]);
		}
    }
}