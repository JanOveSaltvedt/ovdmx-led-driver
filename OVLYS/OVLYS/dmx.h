/*
 * dmx.h
 *
 * Created: 26.10.2013 18:29:24
 *  Author: janov_000
 */ 


#ifndef DMX_H_
#define DMX_H_

#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct {
	uint8_t start_code;
	uint16_t data_length;
	uint8_t data[512];
} dmx_packet_t;

#define RDM_BROADCAST_ALL_MID 0xFFFF
#define RDM_BROADCAST_ALL_DID 0xFFFFFFFF

#define RDM_CMD_DISCOVERY 0x10
#define RDM_CMD_DISCOVERY_RESPONSE 0x11
#define RDM_CMD_GET	0x20
#define RDM_CMD_GET_RESPONSE 0x21
#define RDM_CMD_SET 0x30
#define RDM_CMD_SET_RESPONSE 0x31

#define RDM_RESPONSE_TYPE_ACK 0x00
#define RDM_RESPONSE_TYPE_ACK_TIMER 0x01
#define RDM_RESPONSE_TYPE_NACK_REASON 0x02
#define RDM_RESPONSE_TYPE_ACK_OVERFLOW 0x03

#define RDM_PID_DISC_UNIQUE_BRANCH	0x0001
#define RDM_PID_DISC_MUTE			0x0002
#define RDM_PID_DISC_UN_MUTE		0x0003

//#define RDM_PID_SUPPORTED_PARAMETERS	0x0050
//#define RDM_PID_PARAMETER_DESCRIPTION	0x0051
#define RDM_PID_DEVICE_INFO				0x0060
#define RDM_PID_SOFTWARE_VERSION_LABEL	0x00C0
#define RDM_PID_DMX_START_ADDRESS		0x00F0

typedef struct {
	union {
		uint8_t lower_bound_uid[6];
		struct {
			uint16_t lower_bound_mid;
			uint32_t lower_bound_did;
		};
	};
	union {
		uint8_t upper_bound_uid[6];
		struct {
			uint16_t upper_bound_mid;
			uint32_t upper_bound_did;
		};
	};
} rdm_disc_unique_branch_data_t;

typedef struct {
	uint8_t sub_start_code;
	uint8_t message_length;
	union {
		uint8_t destination_uid[6];
		struct {
			uint16_t destination_mid;
			uint32_t destination_did;
		};
	};
	union {
		uint8_t source_uid[6];
		struct {
			uint16_t source_mid;
			uint32_t source_did;
		};
	};
	
	uint8_t transaction_number;
	uint8_t port_id;
	uint8_t message_count;
	uint16_t sub_device;
	uint8_t command_class;
	
	uint16_t parameter_id;
	uint8_t parameter_data_length;
	union {
		rdm_disc_unique_branch_data_t disc_unique_branch_data;
	};
} rdm_frame_header_t;

void dmx_init(void);
dmx_packet_t* dmx_get_active_packet(void);
uint8_t dmx_get_state(void);

uint8_t dmx_has_new_data(void);

#endif /* DMX_H_ */