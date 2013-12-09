/*
 * device.h
 *
 * Created: 31.10.2013 18:59:39
 *  Author: janov_000
 */ 


#ifndef DEVICE_H_
#define DEVICE_H_

#include <avr/io.h>

void device_init(void);
uint16_t device_get_manufacter_id(void);
uint32_t device_get_device_id(void);
uint8_t device_read_calibration_byte(uint8_t index );

#endif /* DEVICE_H_ */