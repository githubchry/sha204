
#include <stdint.h>
#include <stdbool.h>

#ifndef ATSHA204_CTC_D1_H_
#define ATSHA204_CTC_D1_H_

// ATSHA204 Specific
#include "sha204_lib_return_codes.h"

#define NONCE_PARAM2					((uint16_t) 0x0000)		//nonce param2. always zero
#define HMAC_MODE_EXCLUDE_OTHER_DATA	((uint8_t) 0x00)		//!< HMAC mode excluded other data
#define HMAC_MODE_INCLUDE_OTP_88		((uint8_t) 0x10)		//!< HMAC mode bit   4: include first 88 OTP bits
#define HMAC_MODE_INCLUDE_OTP_64		((uint8_t) 0x20)		//!< HMAC mode bit   5: include first 64 OTP bits
#define HMAC_MODE_INCLUDE_SN			((uint8_t) 0x40)		//!< HMAC mode bit   6: include serial number
#define DERIVE_KEY_RANDOM_NONCE			((uint8_t) 0x00)		//Derive key mode using random nonce
#define MAC_MODE_NO_TEMPKEY				((uint8_t) 0x00)		//MAC mode using internal key and challenge to get MAC result
#define LOCK_PARAM2_NO_CRC				((uint16_t) 0x0000)		//Lock mode : not using checksum to validate the data written
#define CHECKMAC_PASSWORD_MODE			((uint8_t) 0X01)		//CheckMac mode : password check operation



#ifdef __cplusplus
extern "C" {
#endif


//! Topics
extern void atsha204_personalization(int fd);
uint8_t random_challenge_response_authentication(int fd, uint16_t key_id, uint8_t *secret_key_value);

//atsha204_actions
uint8_t atsha204_read_sn(int fd, uint8_t data[9]);
uint8_t atsha204_read_lock(int fd, uint8_t data[4]);
uint8_t atsha204_read_devrev(int fd, uint8_t data[4]);

uint8_t atsha204_read_config(int fd, uint8_t data[88]);
uint8_t atsha204_write_config(int fd, uint8_t data[68]);

uint8_t atsha204_lock_conf(int fd);
uint8_t atsha204_lock_data(int fd);

uint8_t atsha204_read_data(int fd, int slot, uint8_t *read_data);
uint8_t atsha204_write_data(int fd, int slot,  uint8_t *write_data);

uint8_t atsha204_encrypted_read(int fd, uint16_t key_id, uint8_t *key_value, uint16_t slot, uint8_t *readdata);
uint8_t atsha204_encrypted_write(int fd, uint16_t key_id, uint8_t *key_value, uint16_t slot, uint8_t *writedata);


#ifdef __cplusplus
}
#endif

#endif /* ATSHA204_CTC_D1_H_  */
