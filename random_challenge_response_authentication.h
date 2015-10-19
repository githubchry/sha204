/*
 * random_challenge_response_authentication.h
 *
 * Created: 10/9/2013 9:16:10 PM
 *  Author: easanghanwa
 */ 


#ifndef RANDOM_CHALLENGE_RESPONSE_AUTHENTICATION_H_
#define RANDOM_CHALLENGE_RESPONSE_AUTHENTICATION_H_
#include <stdint.h>
#include "sha204_lib_return_codes.h"   // declarations of function return codes
#include "sha204_comm_marshaling.h"
#include "sha204_helper.h"
#include "atsha204_ctc_d1_solutions.h"

uint8_t num_in[20] = {
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
						0x10, 0x11, 0x12, 0x13
};
	
uint8_t secret_key_value[0x20] = {
						0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
						0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
						0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
						0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
};


#endif /* RANDOM_CHALLENGE_RESPONSE_AUTHENTICATION_H_ */
