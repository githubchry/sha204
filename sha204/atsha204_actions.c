#include "atsha204_actions.h"

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
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_idle(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! atsha204_read_config\n");
        return -1;
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
        return -1;
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
        //sha204p_wakeup(fd);
        status = sha204m_execute(fd, &cmd_args);
        //sha204p_idle(fd);
        if (status != SHA204_SUCCESS) {
            printf("FAILED! atsha204_read_config\n");
            return -1;
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
        //sha204p_wakeup(fd);
        status = sha204m_execute(fd, &cmd_args);
        //sha204p_idle(fd);
        if (status != SHA204_SUCCESS) {
            printf("FAILED! atsha204_read_config\n");
            return -1;
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

    // 4字节写17次
    for (int i = 0; i < 17; ++i) {
        // Write the configuration parameters to the slot
        cmd_args.op_code = SHA204_WRITE;
        cmd_args.param_1 = SHA204_ZONE_CONFIG;
        cmd_args.param_2 = DEVICE_MODES_ADDRESS + i;   // 从0x04开始写
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
        //sha204p_wakeup(fd);
        status = sha204m_execute(fd, &cmd_args);
        //sha204p_idle(fd);
        if (status != SHA204_SUCCESS) {
            printf("FAILED! CONF ZONE is locked! \n");
            return -1;
        }
    }

    return SHA204_SUCCESS;
}


/**********************************************************************
*Function	:	atsha204_read_conf
*Arguments	:	int fd				---file description
*				int slot, 				---atsha204a  slot
*				uint8_t *read_conf	---read out two args
*description	:	can read the config at any time
**********************************************************************/
uint8_t atsha204_read_conf(int fd, int slot, uint8_t *read_conf) {
    uint8_t status = SHA204_SUCCESS;
    uint8_t flag = 0;

    if (slot < 0 || slot > 15) {
        return -1;
    }
    flag = slot % 2;

    // Write the configuration parameters to the slot
    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = (uint16_t) (slot / 2 + 5);
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
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_idle(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! a_read_conf\n");
        return -1;
    }
    if (flag == 0) {
        memcpy(read_conf, &global_rx_buffer[1], 0x02);
    } else {
        memcpy(read_conf, &global_rx_buffer[3], 0x02);
    }

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
    if (slot < 0 || slot > 15) { return -1; }
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
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_sleep(fd);
    memcpy(readdata, &global_rx_buffer[1], 0x20);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! a_read_data\n");
        return -1;
    }

    return status;

}

/**********************************************************************
*Function	:	atsha204_write_conf
*Arguments	:	int fd				---file description
*				int slot, 				---atsha204a  slot
*				uint8_t 	conf_low_8_bits,	
						conf_high_8_bits
*description	:	can write config before locking the config zone
**********************************************************************/
uint8_t atsha204_write_conf(int fd, int slot, uint8_t conf_low_8_bits, uint8_t conf_high_8_bits) {
    uint8_t status = SHA204_SUCCESS;
    uint8_t read_conf[2] = {0};
    uint8_t flag = 0;
    uint8_t config_params[4] = {0};

    if (slot < 0 || slot > 15) { return -1; }
    flag = slot % 2;
    if (flag == 0) {
        atsha204_read_conf(fd, slot + 1, read_conf);
        config_params[0] = conf_low_8_bits;
        config_params[1] = conf_high_8_bits;
        config_params[2] = read_conf[0];
        config_params[3] = read_conf[1];
    } else {
        atsha204_read_conf(fd, slot - 1, read_conf);
        config_params[2] = conf_low_8_bits;
        config_params[3] = conf_high_8_bits;
        config_params[0] = read_conf[0];
        config_params[1] = read_conf[1];
    }

    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = (uint16_t) (slot / 2 + 5);
    cmd_args.data_len_1 = SHA204_ZONE_ACCESS_4;
    cmd_args.data_1 = config_params;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x10;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_idle(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! CONF ZONE is locked! \n");
        return -1;
    }

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
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_sleep(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! a_lock_conf\n");
        return -1;
    }
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
    if (slot < 0 || slot > 15) { return -1; }
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
    //sha204p_wakeup(fd);
    status = sha204m_execute(fd, &cmd_args);
    //sha204p_sleep(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! a_write_data\n");
        return -1;
    }
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
    //sha204p_sleep(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! a_lock_data\n");
        return -1;
    }
}

