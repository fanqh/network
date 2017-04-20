#include "aes_128.h"
#include "bsp.h"

#define REG_ADDR_AES_KEY    (0x550)
#define REG_AES_DATA        REG_ADDR32(0x548)
#define REG_AES_CTRL        REG_ADDR8(0x540)
enum {
    FLD_AES_CTRL_CODEC_TRIG = BIT(0),
    FLD_AES_CTRL_DATA_FEED = BIT(1),
    FLD_AES_CTRL_CODEC_FINISHED = BIT(2),
};

/**
 * @brief This function servers to perform aes_128 encryption for 16-Byte input data 
 *         with specific 16-Byte key
 * @param[in]   Key the pointer to the 16-Byte Key
 * @param[in]   Data the pointer to the 16-Byte plain text
 * @param[out]  Result the pointer to the encryption result cipher text
 * @return 0 success, other failure 
 */
int AES_Encrypt(unsigned char *Key, unsigned char *Data, unsigned char *Result)
{
    unsigned int tmp = 0;
    unsigned char *p = Data;
    unsigned char i = 0;
    
    //trig encrypt operation
    REG_AES_CTRL &= (~FLD_AES_CTRL_CODEC_TRIG);

    //set the key
    for (i = 0; i < 16; i++) {
        REG_ADDR8(REG_ADDR_AES_KEY + i) = Key[i];
    }

    //feed the data
    while (REG_AES_CTRL & FLD_AES_CTRL_DATA_FEED) {
        tmp = p[0] + (p[1]<<8) + (p[2]<<16) + (p[3]<<24);
        REG_AES_DATA = tmp;
        p += 4;
    }
    
    //wait for aes ready 
    while ((REG_AES_CTRL & FLD_AES_CTRL_CODEC_FINISHED) == 0);

    //read out the result
    p = Result;
    for (i = 0; i < 4; i++) {
        tmp = REG_AES_DATA;
        *p++ = tmp & 0xff;
        *p++ = (tmp>>8) & 0xff;
        *p++ = (tmp>>16) & 0xff;
        *p++ = (tmp>>24) & 0xff;     
    }
    return 0;
}

/**
 * @brief This function servers to perform aes_128 decryption for 16-Byte input data 
 *         with specific 16-Byte key
 * @param[in]   Key the pointer to the 16-Byte Key
 * @param[in]   Data the pointer to the 16-Byte cipher text
 * @param[out]  Result the pointer to the decryption result plain text
 * @return 0 success, other failure 
 */
int AES_Decrypt(unsigned char *Key, unsigned char *Data, unsigned char *Result)
{
    unsigned int tmp = 0;
    unsigned char *p = Data;
    unsigned char i = 0;

    //trig decrypt operation
    REG_AES_CTRL |= FLD_AES_CTRL_CODEC_TRIG;

    //set the key
    for (i = 0; i < 16; i++) {
        REG_ADDR8(REG_ADDR_AES_KEY + i) = Key[i];
    }

    //feed the data
    while (REG_AES_CTRL & FLD_AES_CTRL_DATA_FEED) {
        tmp = p[0] + (p[1]<<8) + (p[2]<<16) + (p[3]<<24);
        REG_AES_DATA = tmp;
        p += 4;
    }
    
    //wait for aes ready
    while ((REG_AES_CTRL & FLD_AES_CTRL_CODEC_FINISHED) == 0);

    //read out the result
    p = Result;
    for (i = 0; i < 4; i++) {
        tmp = REG_AES_DATA;
        *p++ = tmp & 0xff;
        *p++ = (tmp>>8) & 0xff;
        *p++ = (tmp>>16) & 0xff;
        *p++ = (tmp>>24) & 0xff;     
    }
    return 0;
}
