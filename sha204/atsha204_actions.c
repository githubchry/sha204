#include "atsha204_actions.h"

// ATSHA204 Specific
#include "sha256.h"
#include "atsha204_i2c.h"
#include "sha204_helper.h"
#include "sha204_comm_marshaling.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>

static struct sha204_command_parameters cmd_args;		// Global Generalized Command Parameter
static uint8_t global_tx_buffer[SHA204_CMD_SIZE_MAX];	// Global Transmit Buffer
static uint8_t global_rx_buffer[SHA204_RSP_SIZE_MAX];	// Global Receive Buffer


// 读出加密芯片锁状态, 共4字节
uint8_t atsha204_read_lock(int fd, uint8_t data[4]) {
    uint8_t status = SHA204_SUCCESS;

    // Write the configuration parameters to the slot
    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = 0x15;
    cmd_args.data_len_1 = 0X00;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0x00;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0x00;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x10;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_idle(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! atsha204_read_config\n");
        return status;
    }

    memcpy(data, &global_rx_buffer[1], 4);

    return status;
}

// 读出加密芯片SN, 共9字节
uint8_t atsha204_read_sn(int fd, uint8_t data[9]) {
    uint8_t status = SHA204_SUCCESS;

    // Write the configuration parameters to the slot
    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_CONFIG | SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 = 0;   // 第一次0x00开始读, 第二次0x08开始读
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x10;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_idle(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! atsha204_read_config\n");
        return status;
    }

    memcpy(data, &global_rx_buffer[1], 4);
    memcpy(data + 4, &global_rx_buffer[1] + 8, 4);
    data[8] = global_rx_buffer[1 + 12];

    return status;
}

/**********************************************************************
*Function	:	atsha204_read_devrev
*Arguments	:	int fd				---file description
*				uint8_t data[4]	---output
*description	:	can read the devre at any time
**********************************************************************/
uint8_t atsha204_read_devrev(int fd, uint8_t data[4]) {
    uint8_t status = SHA204_SUCCESS;
    // Use the DevRev command to check communication to chip by validating value received.
    // Note that DevRev value is not constant over future revisions of the chip so failure
    // of this function may not mean bad connection.
    cmd_args.op_code		= SHA204_DEVREV;				// ATSHA204 Command OpCode Parameter
    cmd_args.param_1		= 0x00;							// ATSHA204 Command Param1 Parameter
    cmd_args.param_2		= 0x00;							// ATSHA204 Command Param2 Parameter
    cmd_args.data_len_1		= 0x00;							// Length in bytes of first data content
    cmd_args.data_1			= NULL;							// Pointer to buffer containing first data set
    cmd_args.data_len_2		= 0x00;							// Length in bytes of second data content
    cmd_args.data_2			= NULL;							// Pointer to buffer containing second data set
    cmd_args.data_len_3		= 0x00;							// Length in bytes of third data content
    cmd_args.data_3			= NULL;							// Pointer to buffer containing third data set
    cmd_args.tx_size		= DEVREV_COUNT;					// Size of the transmit buffer
    cmd_args.tx_buffer		= global_tx_buffer;				// Pointer to the transmit buffer
    cmd_args.rx_size		= sizeof(global_rx_buffer);		// Size of the receive buffer
    cmd_args.rx_buffer		= global_rx_buffer;				// Pointer to the receive buffer
    status = sha204m_execute(fd,&cmd_args);						// Marshals the parameters and executes the command

    sha204p_sleep(fd);  // Put the chip to sleep in case you stop to examine buffer contents

    // validate the received value for DevRev
    if( status != SHA204_SUCCESS ) {
        printf("FAILED! atsha204_read_devrev\n");
        return status;
    }

    memcpy(data, &global_rx_buffer[1], 4);

    return status;

}

/**********************************************************************
*Function	:	atsha204_read_config
*Arguments	:	int fd				---file description
*				uint8_t *read_conf	---read out all
*description	:	读出整个config zone, 共88字节
**********************************************************************/
uint8_t atsha204_read_config(int fd, uint8_t data[88]) {
    uint8_t status = SHA204_SUCCESS;

    // 先读两次, 每次32字节  param_1指定SHA204_ZONE_COUNT_FLAG
    for (int i = 0; i < 2; ++i) {
        // Write the configuration parameters to the slot
        cmd_args.op_code = SHA204_READ;
        cmd_args.param_1 = SHA204_ZONE_CONFIG | SHA204_ZONE_COUNT_FLAG;
        cmd_args.param_2 = 8 * i;   // 第一次0x00开始读, 第二次0x08开始读
        cmd_args.data_len_1 = 0;
        cmd_args.data_1 = NULL;
        cmd_args.data_len_2 = 0;
        cmd_args.data_2 = NULL;
        cmd_args.data_len_3 = 0;
        cmd_args.data_3 = NULL;
        cmd_args.tx_size = 0x10;
        cmd_args.tx_buffer = global_tx_buffer;
        cmd_args.rx_size = 0x10;
        cmd_args.rx_buffer = global_rx_buffer;
        sha204p_wakeup(fd);
        status = sha204m_execute(fd, &cmd_args);
        //sha204p_idle(fd);
        if (status != SHA204_SUCCESS) {
            printf("FAILED! atsha204_read_config\n");
            return status;
        }

        memcpy(data + 32 * i, &global_rx_buffer[1], 32);
    }

    // 接着读6次, 每次4字节, param_1不指定SHA204_ZONE_COUNT_FLAG
    for (int i = 0; i < 6; ++i) {
        // Write the configuration parameters to the slot
        cmd_args.op_code = SHA204_READ;
        cmd_args.param_1 = SHA204_ZONE_CONFIG;
        cmd_args.param_2 = 0x10 + i;
        cmd_args.data_len_1 = 0;
        cmd_args.data_1 = NULL;
        cmd_args.data_len_2 = 0;
        cmd_args.data_2 = NULL;
        cmd_args.data_len_3 = 0;
        cmd_args.data_3 = NULL;
        cmd_args.tx_size = 0x10;
        cmd_args.tx_buffer = global_tx_buffer;
        cmd_args.rx_size = 0x10;
        cmd_args.rx_buffer = global_rx_buffer;
        sha204p_wakeup(fd);
        status = sha204m_execute(fd, &cmd_args);
        //sha204p_idle(fd);
        if (status != SHA204_SUCCESS) {
            printf("FAILED! atsha204_read_config\n");
            return status;
        }

        memcpy(data + 64 + i * 4, &global_rx_buffer[1], 4);
    }

    return status;
}

/**********************************************************************
*Function	:	atsha204_write_config
*Arguments	:	int fd				---file description
*				uint8_t data[88], 				---atsha204a  config
*description	:	can write config before locking the config zone
**********************************************************************/
uint8_t atsha204_write_config(int fd, uint8_t data[68]) {

    uint8_t status = SHA204_SUCCESS;

    sha204p_wakeup(fd);
    // 4字节写17次
    for (int i = 0; i < 17; ++i) {
        // Write the configuration parameters to the slot
        cmd_args.op_code = SHA204_WRITE;
        cmd_args.param_1 = SHA204_ZONE_CONFIG;
        cmd_args.param_2 = 0x04 + i;   // 从0x04开始写
        cmd_args.data_len_1 = SHA204_ZONE_ACCESS_4;
        cmd_args.data_1 = data + 4 * i;
        cmd_args.data_len_2 = 0;
        cmd_args.data_2 = NULL;
        cmd_args.data_len_3 = 0;
        cmd_args.data_3 = NULL;
        cmd_args.tx_size = 0x10;
        cmd_args.tx_buffer = global_tx_buffer;
        cmd_args.rx_size = 0x10;
        cmd_args.rx_buffer = global_rx_buffer;
        status = sha204m_execute(fd, &cmd_args);
        sha204p_idle(fd);
        if (status != SHA204_SUCCESS) break;
    }

    sha204p_sleep(fd);
    return status;
}


/**********************************************************************
*Function	:	atsha204_read_data
*Arguments	:	int fd				---file description
*				int slot, 				---atsha204a  slot
*				uint8_t *readdata		---read out 32 bytes key
*description	:	seem to nothing after chip locked
**********************************************************************/
uint8_t atsha204_read_data(int fd, int slot, uint8_t *readdata) {
    uint8_t status = SHA204_SUCCESS;
    if (slot < 0 || slot > 15) { return SHA204_BAD_PARAM; }
    uint16_t slot_addr = (uint16_t) (slot * 8);

    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_DATA | SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 = slot_addr;
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x30;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    sha204p_sleep(fd);
    memcpy(readdata, &global_rx_buffer[1], 0x20);

    return status;
}

/**********************************************************************
*Function	:	atsha204_lock_conf
*Arguments	:	int fd				---file description
*description	:	lock the config zone ,
				after this action can write data successfully
**********************************************************************/
uint8_t atsha204_lock_conf(int fd) {
    uint8_t status = SHA204_SUCCESS;
    // **** LOCK THE CONFIGURATION ZONE.

    // Perform the configuration lock:
    cmd_args.op_code = SHA204_LOCK;
    cmd_args.param_1 = LOCK_ZONE_NO_CRC;
    cmd_args.param_2 = LOCK_PARAM2_NO_CRC;
    cmd_args.data_len_1 = 0X00;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x10;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    sha204p_sleep(fd);
    return status;
}

/**********************************************************************
*Function	:	atsha204_write_data
*Arguments	:	int fd				---file description
*				int slot, 				---atsha204a  slot
*				uint8_t *write_data	---write in 32 bytes key
*description	:	after lock the config can do this action successfully
				and  can NOT write anymore after LOCK the data zone
**********************************************************************/
uint8_t atsha204_write_data(int fd, int slot, uint8_t *write_data) {
    uint8_t status = SHA204_SUCCESS;
    if (slot < 0 || slot > 15) { return SHA204_BAD_PARAM; }
    uint16_t slot_addr = (uint16_t) (slot * 8);

    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_DATA | SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 = slot_addr;
    cmd_args.data_len_1 = SHA204_ZONE_ACCESS_32;
    cmd_args.data_1 = write_data;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    sha204p_sleep(fd);

    return status;
}

/**********************************************************************
*Function	:	atsha204_lock_data
*Arguments	:	int fd				---file description
*description	:	be carefully running this action
				because almost action be disable after it done
**********************************************************************/
uint8_t atsha204_lock_data(int fd) {
    uint8_t status = SHA204_SUCCESS;
    cmd_args.op_code = SHA204_LOCK;
    cmd_args.param_1 = LOCK_ZONE_NO_CONFIG | LOCK_ZONE_NO_CRC;
    cmd_args.param_2 = LOCK_PARAM2_NO_CRC;
    cmd_args.data_len_1 = 0X00;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x10;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    sha204p_sleep(fd);
    return status;
}

//======================================================================================================================

uint8_t read_num_in[20] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13
};

uint8_t atsha204_encrypted_read(int fd, uint16_t key_id, uint8_t *key_value,uint16_t slot, uint8_t *readdata) {
    int i;
    static uint8_t status = SHA204_SUCCESS;
    static uint8_t tmpdata[0x20] = {0};
    static uint8_t random_number[0x20] = {0};		// Random number returned by Random NONCE command
    static uint8_t computed_response[0x20] = {0};	// Host computed expected response

    struct sha204h_decrypt_in_out decrypt_param;	// Parameter for decrypt helper function
    struct sha204h_nonce_in_out nonce_param;		// Parameter for nonce helper function
    struct sha204h_gen_dig_in_out gendig_param;	// Parameter for gen_dig helper function
    struct sha204h_temp_key computed_tempkey;		// TempKey parameter for nonce and gen_dig helper function

    //add by jli :That before every executing  cmd sent to ATSHA204 chip  should have waked it up once!
    sha204p_wakeup(fd);

    printf("ATSHA204A encrypted read  !\n");
    //nonce operation
    cmd_args.op_code = SHA204_NONCE;
    cmd_args.param_1 = NONCE_MODE_SEED_UPDATE;
    cmd_args.param_2 = NONCE_PARAM2;
    cmd_args.data_len_1 = NONCE_NUMIN_SIZE;
    cmd_args.data_1 = read_num_in;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = NONCE_COUNT_SHORT;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = NONCE_RSP_SIZE_LONG;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    //sha204p_idle(fd);
    if(status != SHA204_SUCCESS) { printf(" Mathine NONCE  FAILED! \n"); return status; }
    // Capture the random number from the NONCE command if it were successful
    memcpy(random_number,&global_rx_buffer[1],0x20);

    //host nonce operation
    nonce_param.mode = NONCE_MODE_SEED_UPDATE;
    nonce_param.num_in = read_num_in;
    nonce_param.rand_out = random_number;
    nonce_param.temp_key = &computed_tempkey;
    status = sha204h_nonce(nonce_param);
    if(status != SHA204_SUCCESS) { printf("HOST   NONCE  FAILED! \n"); return status; }

    //gendig operation
    cmd_args.op_code = SHA204_GENDIG;
    cmd_args.param_1 = GENDIG_ZONE_DATA;
    cmd_args.param_2 = key_id;
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = SHA204_CMD_SIZE_MIN;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = GENDIG_RSP_SIZE;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("Mathine  GENGID  FAILED! \n"); return status; }

    //Host gengid operation
    memcpy(tmpdata,key_value,0x20);
    gendig_param.zone = GENDIG_ZONE_DATA;
    gendig_param.key_id = key_id;
    gendig_param.stored_value = tmpdata;
    gendig_param.temp_key = &computed_tempkey;
    status = sha204h_gen_dig(gendig_param);
    if(status != SHA204_SUCCESS) { printf("HOST   GENDIG  FAILED! \n"); return status; }

    //Read operation
    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 =  (uint16_t)(slot * 8);
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x30;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    memcpy(tmpdata,&global_rx_buffer[1],0x20);
    if(status != SHA204_SUCCESS) { printf("FAILED! e_read_data\n"); return status ; }

    decrypt_param.data = tmpdata;
    decrypt_param.temp_key = &computed_tempkey;
    status = sha204h_decrypt(decrypt_param);
    if(status != SHA204_SUCCESS) { printf("HOST   DECRYPT  FAILED! \n"); return status; }
    memcpy(readdata,tmpdata,0x20);
    return status;
}

//======================================================================================================================

uint8_t write_num_in[20] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13
};

uint8_t atsha204_encrypted_write(int fd, uint16_t key_id, uint8_t *key_value, uint16_t slot, uint8_t *writedata) {
    int i;
    static uint8_t status = SHA204_SUCCESS;
    static uint8_t tmpdata[0x20] = {0};
    static uint8_t random_number[0x20] = {0};		// Random number returned by Random NONCE command
    static uint8_t computed_response[0x20] = {0};	// Host computed expected response
    static uint8_t host_mac[0x20] = {0};
    struct sha204h_encrypt_in_out encrypt_param;	//Parameter for encrypt helper function
    struct sha204h_nonce_in_out nonce_param;		// Parameter for nonce helper function
    struct sha204h_gen_dig_in_out gendig_param;	// Parameter for gen_dig helper function
    struct sha204h_temp_key computed_tempkey;		// TempKey parameter for nonce and gen_dig helper function

    //add by jli :That before every executing  cmd sent to ATSHA204 chip  should have waked it up once!
    sha204p_wakeup(fd);

    printf("ATSHA204A encrypted write  !\n");
    //nonce operation
    cmd_args.op_code = SHA204_NONCE;
    cmd_args.param_1 = NONCE_MODE_SEED_UPDATE;
    cmd_args.param_2 = NONCE_PARAM2;
    cmd_args.data_len_1 = NONCE_NUMIN_SIZE;
    cmd_args.data_1 = write_num_in;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = NONCE_COUNT_SHORT;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = NONCE_RSP_SIZE_LONG;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    //sha204p_idle(fd);
    if(status != SHA204_SUCCESS) { printf(" Mathine NONCE  FAILED! \n"); return status; }
    // Capture the random number from the NONCE command if it were successful
    memcpy(random_number,&global_rx_buffer[1],0x20);

    //host nonce operation
    nonce_param.mode = NONCE_MODE_SEED_UPDATE;
    nonce_param.num_in = write_num_in;
    nonce_param.rand_out = random_number;
    nonce_param.temp_key = &computed_tempkey;
    status = sha204h_nonce(nonce_param);
    if(status != SHA204_SUCCESS) { printf("HOST   NONCE  FAILED! \n");  return status; }

    //Host gengid operation
    memcpy(tmpdata,key_value,0x20);
    gendig_param.zone = GENDIG_ZONE_DATA;
    gendig_param.key_id = key_id;
    gendig_param.stored_value = tmpdata;
    gendig_param.temp_key = &computed_tempkey;
    status = sha204h_gen_dig(gendig_param);
    if(status != SHA204_SUCCESS) { printf("HOST   GENDIG  FAILED! \n");  return status; }

    //gendig operation
    cmd_args.op_code = SHA204_GENDIG;
    cmd_args.param_1 = GENDIG_ZONE_DATA;
    cmd_args.param_2 = key_id;
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = SHA204_CMD_SIZE_MIN;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = GENDIG_RSP_SIZE;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("Mathine  GENGID  FAILED! \n");  return status; }

    //Host XOR operation
    memcpy(tmpdata,writedata,0x20);
    encrypt_param.temp_key = &computed_tempkey;
    encrypt_param.zone = SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG;
    encrypt_param.address = (uint16_t)(slot * 8);
    encrypt_param.data = tmpdata;
    encrypt_param.mac = host_mac;
    status = sha204h_encrypt(encrypt_param);
    if(status != SHA204_SUCCESS) { printf("HOST   ENCRYPT  FAILED! \n");  return status; }

    //Write operation
    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 =  (uint16_t)(slot * 8);
    cmd_args.data_len_1 = SHA204_ZONE_ACCESS_32;
    cmd_args.data_1 = tmpdata;
    cmd_args.data_len_2 = SHA204_ZONE_ACCESS_32;
    cmd_args.data_2 = encrypt_param.mac;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x30;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204p_wakeup(fd);
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! e_write_data\n");  return status ; }

    return status;

}

//======================================================================================================================

uint8_t num_in[20] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13
};

uint8_t Secrect_key_value[0x20] = {
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
        0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
};
uint8_t random_challenge_response_authentication(int fd, uint16_t key_id, uint8_t *secret_key_value) {

    static uint8_t status = SHA204_SUCCESS;
    static uint8_t random_number[0x20] = {0};		// 随机 NONCE 命令返回的随机数 Random number returned by Random NONCE command
    static uint8_t computed_response[0x20] = {0};	// 主机计算的预期响应 Host computed expected response
    static uint8_t atsha204_response[0x20] = {0};	// 从ATSHA204设备收到的实际响应 Actual response received from the ATSHA204 device
    struct sha204h_nonce_in_out nonce_param;		// nonce辅助函数参数 Parameter for nonce helper function
    struct sha204h_mac_in_out mac_param;			// mac辅助函数参数 Parameter for mac helper function
    struct sha204h_temp_key computed_tempkey;		// 用于 nonce 和 mac 辅助函数的 TempKey 参数 TempKey parameter for nonce and mac helper function

    //在每次向 ATSHA204 芯片发送执行命令之前，都应该唤醒它一次！
    sha204p_wakeup(fd);

    printf("Random Chal_Response r\n");

    // 注意事项:
    // 1. 随机质询-响应过程涉及在每次认证过程中使用随机质询。具有良好随机数生成源的主机可以简单地确保质询是随机的，同时进行一个简单的 MAC 命令调用。
    // 2. 没有良好随机数生成器的主机可能会倾向于使用 ATSHA204 的随机命令从设备中获取随机数。虽然通过这个过程获得的随机数是最高质量的，但该过程易受中间人攻击的影响，攻击者可以简单地拦截随机数并发送一个非随机数给主机。
    // 3. 没有良好随机数生成器并希望避免上述注意事项#2中描述的中间人攻击的主机系统可以使用涉及 ATSHA204 在随机模式下的 NONCE 命令的认证过程。NONCE 命令保证了 ATSHA204 设备内部的随机状态，这几乎是不可能伪造的。在此练习中示例的就是这种过程。


    // *** 第一步：发出一个没有 EEPROM 种子更新的 NONCE ***
    //				NONCE 命令在 ATSHA204 设备中生成一个内部随机状态。请注意，实际的随机 NONCE 是使用内部生成的随机数和其他设备参数计算得出的值。
    //				NONCE 命令发出这个随机值供主机在主机端计算等效的 NONCE。捕获这个随机数并保留，以便在主机端计算等效的 NONCE。
    cmd_args.op_code = SHA204_NONCE;
    cmd_args.param_1 = NONCE_MODE_NO_SEED_UPDATE;
    cmd_args.param_2 = NONCE_PARAM2;
    cmd_args.data_len_1 = NONCE_NUMIN_SIZE;
    cmd_args.data_1 = num_in;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = NONCE_COUNT_SHORT;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = NONCE_RSP_SIZE_LONG;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    //sha204p_idle(fd);
    if(status != SHA204_SUCCESS) { printf(" Mathine NONCE  FAILED! \n"); return status; }

    // Capture the random number from the NONCE command if it were successful
    memcpy(random_number,&global_rx_buffer[1],0x20);

    // *** STEP 2:	COMPUTE THE EQUIVALENT NONCE ON THE HOST SIDE
    //
    //				Go the easy way using the host helper functions provided with
    //				the ATSHA204 library.

    nonce_param.mode = NONCE_MODE_NO_SEED_UPDATE;
    nonce_param.num_in = num_in;
    nonce_param.rand_out = random_number;
    nonce_param.temp_key = &computed_tempkey;
    status = sha204h_nonce(nonce_param);
    if(status != SHA204_SUCCESS) { printf("HOST   NONCE  FAILED! \n"); return status; }


    // *** STEP 3:	ISSUE THE MAC COMMAND
    //
    //				Starting from a randomized internal state of the ATSHA204 device
    //				guarantees that this MAC command call is executing a random
    //				challenge-response authentication.

    // Issue the MAC command
    cmd_args.op_code = SHA204_MAC;
    cmd_args.param_1 = MAC_MODE_BLOCK2_TEMPKEY;
    cmd_args.param_2 = key_id;
    cmd_args.data_len_1 = 0;
    cmd_args.data_1 = NULL;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = MAC_COUNT_SHORT;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = MAC_RSP_SIZE;
    cmd_args.rx_buffer = global_rx_buffer;
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("Mathine  MACFAILED! \n"); return status; }

    // Capture actual response from the ATSHA204 device
    memcpy(atsha204_response,&global_rx_buffer[1],0x20);


    // *** STEP 4:	DYNAMICALLY VALIDATE THE (MAC) RESPONSE
    //
    //				Note that this requires knowledge of the actual secret key
    //				value.

    mac_param.mode = MAC_MODE_BLOCK2_TEMPKEY;
    mac_param.key_id = key_id;
    mac_param.challenge = NULL;
    mac_param.key = secret_key_value;
    mac_param.otp = NULL;
    mac_param.sn = NULL;
    mac_param.response = computed_response;
    mac_param.temp_key = &computed_tempkey;
    status = sha204h_mac(mac_param);
    if(status != SHA204_SUCCESS) { printf("HOST   MAC  FAILED! \n"); return status; }


    // Moment of truth: Compare the received response with the dynamically computed expected response.
    if (0 == memcmp(computed_response,atsha204_response,0x20)) {
        printf("Authentication  SUCCESS!\n");
        return SHA204_SUCCESS;
    } else {
        printf("Authentication   FAILED! \n");
        return -1;
    }
}
