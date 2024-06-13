/*
 * atsha204_i2c.h
 *
 * Created: 10/6/2013 5:28:46 AM
 *  Author: easanghanwa
 */ 


#ifndef ATSHA204_I2C_H_
#define ATSHA204_I2C_H_


#include <stdint.h>                                  // data type definitions

#define SHA204_BUFFER_POS_COUNT      (0)             //!< buffer index of count byte in command or response
#define SHA204_BUFFER_POS_DATA       (1)             //!< buffer index of data in response

//! width of Wakeup pulse in 10 us units
#define SHA204_WAKEUP_PULSE_WIDTH    (uint8_t) (6.0 * CPU_CLOCK_DEVIATION_POSITIVE + 0.5)

//! delay between Wakeup pulse and communication in ms
#define SHA204_WAKEUP_DELAY          (uint8_t) (3.0 * CPU_CLOCK_DEVIATION_POSITIVE + 0.5)


uint8_t sha204p_send_command(int fd,uint8_t count, uint8_t *command);
uint8_t sha204p_receive_response(int fd,uint8_t size, uint8_t *response);
void    sha204p_init(void);
void    sha204p_set_device_id(uint8_t id);
uint8_t sha204p_wakeup(int fd);
uint8_t sha204p_idle(int fd);
uint8_t sha204p_sleep(int fd);
uint8_t sha204p_reset_io(int fd);
uint8_t sha204p_resync(int fd,uint8_t size, uint8_t *response);



#endif /* ATSHA204_I2C_H_ */
