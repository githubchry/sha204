
/*
 * $safeprojectname$.c
 *
 * Created: 7/10/2013 1:56:24 PM
 *  Author: easanghanwa
  * Modified:jli@acorn-net.com
 */

#include <sstream>  // std::ostringstream
#include <iomanip>

#include "sha204/atsha204_actions.h"


#define I2C_BUS       "/dev/i2c-c"
#define ATSHA204_ADDR  0x64


void dump_config(uint8_t data[88]) {

    // 解析成字符串
    std::ostringstream ss;

    ss << "SN:\n" << std::hex << std::setfill('0');
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 4; ++j)
            ss << std::setw(2) << int(data[i * 4 + j]) << " ";
        ss << "\n";
    }

    ss << "\nSlotConfig:\n";
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 4; ++j)
            ss << std::setw(2) << int(data[20 + i * 4 + j]) << " ";
        ss << "\n";
    }

    ss << "\nUseFlag:\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            ss << std::setw(2) << int(data[52 + i * 4 + j]) << " ";
        ss << "\n";
    }

    ss << "\nLastKeyUse:\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
            ss << std::setw(2) << int(data[68 + i * 4 + j]) << " ";
        ss << "\n";
    }

    ss << "\nLock:\n";

    for (int i = 0; i < 4; ++i)
        ss << std::setw(2) << int(data[84 + i]) << " ";

    ss << "\n";

    printf("%s\n", ss.str().c_str());
}


/*
void atsha204_personalization(int fd) {
    static uint8_t status = SHA204_SUCCESS;
    static uint8_t config_params[0x04] = {0};
    static uint8_t slot_content [0x20] = {0};
    uint8_t i = 0;


    printf("PERSONALIZATION!_!\n");


    // **** EXERCISE: SLOTS 0 & 1 CONFIGURATION
    // Configure slots 0 and 1 as follows:
    //		- Slot 0 for storage of a non-readable and non-modifiable key.
    //		- Program slot 0 key to hex: 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55
    //		- Slot 1 for storage of a modifiable and encrypted-readable key.
    //		- Program slot 1 key to hex: 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11

    // Decide the slot configuration parameters
    config_params[0] = 0x80;	// Slot 0 : IsSecret, Read Key = 0x00
    config_params[1] = 0x80;	// Slot 0 : Write Never, Write Key = 0x00;
    config_params[2] = 0xC0;	// Slot 1 : Is Secret, Encrypted Readable, Read Key = 0x00
    config_params[3] = 0xF0;	// Slot 1 : Encrypted writes, Write Key = 0x00

    // Write the configuration parameters to the slot
    atsha204_write_conf(fd, 0, 0x80, 0x80);

    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = SLOT_CONFIG_0_1_ADDRESS;
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
    sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) {printf("FAILED! p_1\n"); return; }



    // **** EXERCISE: SLOTS 2 & 3 CONFIGURATION
    // Configure slots 2 and 3 as follows:
    //		- Slot 2 for encrypted reads and encrypted writes. Use key 1 for reads, key 0 for writes.
    //		- Leave factory default content as initial value in slot 2.
    //		- Slot 3 for encrypted reads and clear writes. Use key 1 for reads.
    //		- Leave factory default content as initial value in slot 3

    // Decide the slot configuration parameters
    config_params[0] = 0x41;	// Slot 2 : Encrypted Reads, Read Key = 0x01
    config_params[1] = 0x40;	// Slot 2 : Encrypted Writes, Write Key = 0x00;
    config_params[2] = 0x41;	// Slot 3 : Encrypted Reads, Read Key = 0x01
    config_params[3] = 0x00;	// Slot 3 : Clear writes, Write Key = 0x00

    // Write the configuration parameters to the slot
    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = SLOT_CONFIG_2_3_ADDRESS;
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
    sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! p_2\n"); return; }


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
    sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) {printf("FAILED p_3!\n"); return; }



    // **** INITIALIZE SLOT CONTENT

    // Slot 0: Program Initial Content
    for(i=0x00; i<0x20; i++) {
        slot_content[i] = 0x55;
    }
    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 = SLOT_0_ADDRESS;
    cmd_args.data_len_1 = SHA204_ZONE_ACCESS_32;
    cmd_args.data_1 = slot_content;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! p_4\n"); return; }



    // Slot 1: Program Initial Content
    for(i=0x00; i<0x20; i++) {
        slot_content[i] = 0x11;
    }
    cmd_args.op_code = SHA204_WRITE;
    cmd_args.param_1 = SHA204_ZONE_DATA|SHA204_ZONE_COUNT_FLAG;
    cmd_args.param_2 = SLOT_1_ADDRESS;
    cmd_args.data_len_1 = SHA204_ZONE_ACCESS_32;
    cmd_args.data_1 = slot_content;
    cmd_args.data_len_2 = 0;
    cmd_args.data_2 = NULL;
    cmd_args.data_len_3 = 0;
    cmd_args.data_3 = NULL;
    cmd_args.tx_size = 0x30;
    cmd_args.tx_buffer = global_tx_buffer;
    cmd_args.rx_size = 0x10;
    cmd_args.rx_buffer = global_rx_buffer;
    sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! p_5\n"); return; }



    // **** LOCK THE DATA REGION.

    // Perform the configuration lock:
    status = atsha204_lock_data(fd);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! p_6\n"); return; }

    // Verify Complete Lock By Inspecting the LOCK CONFIG and LOCK VALUE registers
    // Perform the configuration lock:
    cmd_args.op_code = SHA204_READ;
    cmd_args.param_1 = SHA204_ZONE_CONFIG;
    cmd_args.param_2 = EXTRA_SELECTOR_LOCK_ADDRESS;
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
    status = sha204m_execute(fd,&cmd_args);
    sha204p_sleep(fd);
    if(status != SHA204_SUCCESS) { printf("FAILED! p_7\n"); return; }





    if((global_rx_buffer[0x03] != 0x00) || (global_rx_buffer[0x04] != 00)) { printf("FAILED! p_8\n"); return; }

    printf("SUCCESSFUL! p_00\n");


    return;
}

*/

int main(int argc, char *argv[]) {
    int fd, i, j;
    static uint8_t status = SHA204_SUCCESS;
    uint8_t serect[32] = {0};    //the key of slot 0
    uint8_t tmp_conf[2];
    uint8_t tmp_key[32];
    uint8_t key_15[32] = {
            0xff, 0xff, 0x68, 0xb7, 0xb8, 0x01, 0xbe, 0x66,
            0x2C, 0xec, 0x74, 0x68, 0x0F, 0xe4, 0x7D, 0xc1,
            0xc6, 0x72, 0x54, 0x3A, 0xe5, 0xbe, 0xda, 0x2e,
            0x91, 0x9A, 0xe5, 0x0D, 0x32, 0xa1, 0xff, 0xff
    };

    uint8_t key_0[0x20] = {
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
    };

    if ((fd = open(I2C_BUS, O_RDWR)) < 0) {
        printf("Unable to open i2c control file");
        exit(1);
    }

    if (ioctl(fd, I2C_SLAVE, ATSHA204_ADDR) < 0) {
        printf("Set chip address failed\n");
    }


    atsha204_read_data(fd, 0, tmp_key);
    printf("read atsha204a slot %d:", 0);
    for (int i = 0; i < SHA204_KEY_SIZE; i++)printf(" %02x", tmp_key[i]);
    printf("\n");


    uint8_t config[88];
    atsha204_read_config(fd, config);

    dump_config(config);

    return 0;


    encrypted_write(fd, 0, serect, 4, key_0);
    encrypted_read(fd, 0, serect, 4, tmp_key);
    for (i = 0; i < 32; i++) {
        printf("%02x", tmp_key[i]);
        if (31 == i) printf("\n");
    }
    encrypted_write(fd, 0, serect, 4, key_15);
    encrypted_read(fd, 0, serect, 4, tmp_key);
    for (i = 0; i < 32; i++) {
        printf("%02x", tmp_key[i]);
        if (31 == i) printf("\n");
    }
    //atsha204_read_data(fd,1,tmp_key);
    //atsha204_write_data(fd,10,key_15);
    //atsha204_read_conf(fd, 15, tmp_conf);
    //atsha204_write_conf(fd,15,0,0);
    //atsha204_lock_conf(fd);
    //atsha204_lock_data(fd);

    //atsha204_DevRev_cmd(fd);

    //atsha204_personalization(fd);

    //random_challenge_response_authentication(fd,15,key_15);
    close(fd);

    return 0;
}

