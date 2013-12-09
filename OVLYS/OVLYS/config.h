/*
 * config.h
 *
 * Created: 29.10.2013 11:13:26
 *  Author: janov_000
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>

#define CONFIG_NUM_CHANNELS 15
typedef struct {
	uint16_t manufacter_id;
	uint32_t device_id;
	int16_t channel_mapping[CONFIG_NUM_CHANNELS];
} config_t;

void config_init(void);
void config_set_default(void);
void config_load(void);
void config_save(void);
config_t* config_get(void);



#endif /* CONFIG_H_ */