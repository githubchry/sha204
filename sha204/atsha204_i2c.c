/*
 * atsha204_i2c.c
 *
 * Created: 10/6/2013 5:28:22 AM
 *  Author: easanghanwa
  * Modified:jli@acorn-net.com
 */

#include "atsha204_i2c.h"
#include "sha204_comm.h"
#include "sha204_lib_return_codes.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>          // errno
#include <string.h>         // strerror

enum i2c_word_address {
    SHA204_I2C_PACKET_FUNCTION_RESET,  //!< Reset device.
    SHA204_I2C_PACKET_FUNCTION_SLEEP,  //!< Put device into Sleep mode.
    SHA204_I2C_PACKET_FUNCTION_IDLE,   //!< Put device into Idle mode.
    SHA204_I2C_PACKET_FUNCTION_NORMAL  //!< Write / evaluate data that follow this word address byte.
};

uint8_t sha204p_wakeup(int fd) {
    unsigned char wakeup = 0;
    write(fd, &wakeup, 1);
    usleep(3 * 1000);   // 唤醒后至少等待2.5ms

    return SHA204_SUCCESS;
}

static uint8_t sha204p_send(int fd, uint8_t word_address, uint8_t count, uint8_t *buffer) {
    unsigned char *r;
    unsigned char *array = (unsigned char *) malloc((count + 1) * sizeof(unsigned char));


    array[0] = word_address;
    memcpy(array + 1, buffer, count);

    int ret = write(fd, array, count + 1);

    printf("iic send [");
    for (int i = 0; i < count + 1; ++i) printf("%02x ", array[i]);
    ret == count + 1 ? printf("] => %d \n", ret) : printf("] => %s \n", strerror(errno));

    free(array);
    return (ret == count + 1) ? SHA204_SUCCESS : ret;
}


uint8_t sha204p_send_command(int fd, uint8_t count, uint8_t *command) {
    return sha204p_send(fd, SHA204_I2C_PACKET_FUNCTION_NORMAL, count, command);
}


uint8_t sha204p_idle(int fd) {
    return sha204p_send(fd, SHA204_I2C_PACKET_FUNCTION_IDLE, 0, NULL);
}


uint8_t sha204p_sleep(int fd) {
    return sha204p_send(fd, SHA204_I2C_PACKET_FUNCTION_SLEEP, 0, NULL);
}


uint8_t sha204p_reset_io(int fd) {
    return sha204p_send(fd, SHA204_I2C_PACKET_FUNCTION_RESET, 0, NULL);
}


uint8_t sha204p_receive_response(int fd, uint8_t size, uint8_t *response) {
    unsigned char count;
    unsigned char *p;

    read(fd, &response[0], 1);

    count = response[0];
    if ((count < SHA204_RSP_SIZE_MIN) || (count > SHA204_RSP_SIZE_MAX))
        return SHA204_INVALID_SIZE;

    int ret = read(fd, response + 1, count - 1);

    printf("iic recv [   ");
    for (int i = 0; i < count; ++i) printf("%02x ", response[i]);
    ret == count - 1 ? printf("] => %d \n", count) : printf("] => %s \n", strerror(errno));

    return (ret > count - 1) ? SHA204_SUCCESS : ret;
}

uint8_t sha204p_resync(int fd, uint8_t size, uint8_t *response) {
    usleep(100 * 1000);
    return SHA204_SUCCESS;
}

