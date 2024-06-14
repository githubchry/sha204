
/*
 * $safeprojectname$.c
 *
 * Created: 7/10/2013 1:56:24 PM
 *  Author: easanghanwa
  * Modified:jli@acorn-net.com
 */

#include "sha204/atsha204_actions.h"

#include <fcntl.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>             // close/write/read
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <sstream>  // std::ostringstream
#include <iomanip>


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


//*
void atsha204_init(int fd) {
    uint8_t status = SHA204_SUCCESS;


    // 验证锁定状态
    uint8_t lock_status[4];
    if (SHA204_SUCCESS != atsha204_read_lock(fd, lock_status)) return;

    if ((lock_status[0x02] == 0x00) && (lock_status[0x03] == 00)) {
        printf("加密芯片已锁定!\n");
        return;
    }

    uint8_t defconfig[68] = {
            0xc8, 0x00, 0x55, 0x00,  //I2C_Addr  CheckMacConfig  OTP_Mode  SelectorMode

            // slot0存密钥,不可读写;slot1存密钥,可加密读写;
            0x80, 0x80, 0xC0, 0xF0,  //SlotConfig  0  1
            // slot2根据slot1加密读, 根据slot0加密写;slot3根据slot1加密读, 明文写
            0x41, 0x40, 0x41, 0x00,  //SlotConfig  2  3
            // slot4/5设置成密钥区，不能进行任何读写，可以执行所有加密命令，无限次使用
            0x80, 0xA0, 0x80, 0xA0,  //SlotConfig  4  5
            // 剩下的全部搞成明文读写
            0x00, 0x00, 0x00, 0x00,  //SlotConfig  6  7
            0x00, 0x00, 0x00, 0x00,  //SlotConfig  8  9     8存放SN
            0x00, 0x00, 0x00, 0x00,  //SlotConfig 10 11
            0x00, 0x00, 0x00, 0x00,  //SlotConfig 12 13
            0x00, 0x00, 0x00, 0x00,  //SlotConfig 14 15

            0xff, 0x00, 0xff, 0x00,  //UseFlag UpdateCount 0 1
            0xff, 0x00, 0xff, 0x00,  //UseFlag UpdateCount 2 3
            0xff, 0x00, 0xff, 0x00,  //UseFlag UpdateCount 4 5
            0xff, 0x00, 0xff, 0x00,  //UseFlag UpdateCount 6 7

            0xff, 0xff, 0xff, 0xff,  //LastKeyUse  0 - 3
            0xff, 0xff, 0xff, 0xff,  //LastKeyUse  4 - 7
            0xff, 0xff, 0xff, 0xff,  //LastKeyUse  8 -11
            0xff, 0xff, 0xff, 0xff   //LastKeyUse 12 -15
    };

    status = atsha204_write_config(fd, defconfig);
    if (status != SHA204_SUCCESS) {
        printf("FAILED p_2!\n");
        return;
    }

    // **** LOCK THE CONFIGURATION ZONE.
    status = atsha204_lock_conf(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED p_3!\n");
        return;
    }


    // 锁定data区域前, 根据计划先写入密钥
    uint8_t slot_content[0x20] = {0};
    // Slot 0: Program Initial Content
    memset(slot_content, 0, sizeof(slot_content));
    memcpy(slot_content, "3wlink.cn", 9);
    status = atsha204_write_data(fd, 0, slot_content);


    memset(slot_content, 0, sizeof(slot_content));
    memcpy(slot_content, "GgsDdu.2017", 11);
    status = atsha204_write_data(fd, 4, slot_content);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! p_4\n");
        return;
    }


    memset(slot_content, 0, sizeof(slot_content));
    memcpy(slot_content, "Admin_123", 9);
    status = atsha204_write_data(fd, 5, slot_content);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! p_4\n");
        return;
    }


    //
    // **** LOCK THE DATA REGION. 锁定data区域
    status = atsha204_lock_data(fd);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! p_6\n");
        return;
    }

    // 再次验证锁定状态
    status = atsha204_read_lock(fd, lock_status);
    if (status != SHA204_SUCCESS) {
        printf("FAILED! p_7\n");
        return;
    }
    if ((lock_status[0x02] != 0x00) || (lock_status[0x03] != 00)) {
        printf("FAILED! p_8\n");
        return;
    }

    printf("SUCCESSFUL! p_00\n");

    return;
}

//*/

int main(int argc, char *argv[]) {
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

    int fd = open(I2C_BUS, O_RDWR);
    if (fd < 0) {
        printf("Unable to open i2c control file");
        exit(1);
    }

    if (ioctl(fd, I2C_SLAVE, ATSHA204_ADDR) < 0) {
        printf("Set chip address failed\n");
    }

    atsha204_init(fd);


    uint8_t config[88];
    uint8_t status = atsha204_read_config(fd, config);
    //assert_param_return(SHA204_SUCCESS == status, -1);
    dump_config(config);

    uint8_t sn[9];
    status = atsha204_read_sn(fd, sn);
    printf("SN:");
    for (int i = 0; i < 9; i++)printf(" %02x", sn[i]);
    printf("\n");


    for (int i = 0; i < 16; i++){
        memset(tmp_key, 0, sizeof(tmp_key));
        status = atsha204_read_data(fd, i, tmp_key);
        printf("SLOT %d data: %s\n", i, tmp_key);
    }
    return 0;


    atsha204_encrypted_write(fd, 0, serect, 4, key_0);
    atsha204_encrypted_read(fd, 0, serect, 4, tmp_key);
    for (int i = 0; i < 32; i++) {
        printf("%02x", tmp_key[i]);
        if (31 == i) printf("\n");
    }
    atsha204_encrypted_write(fd, 0, serect, 4, key_15);
    atsha204_encrypted_read(fd, 0, serect, 4, tmp_key);
    for (int i = 0; i < 32; i++) {
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

