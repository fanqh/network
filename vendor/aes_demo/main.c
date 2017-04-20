#include "../../drivers.h"
#include "../../common.h"

unsigned long firmwareVersion;

static void SYS_Init(void)
{
    BSP_SysCtlTypeDef SysCtrl;
    SysCtrl.rst0 = (~FLD_RST0_ALL);
    SysCtrl.rst1 = (~FLD_RST1_ALL);
    SysCtrl.rst2 = (~FLD_RST2_ALL);
    SysCtrl.clk0 = FLD_CLK0_EN_ALL;
    SysCtrl.clk1 = FLD_CLK1_EN_ALL;
    SysCtrl.clk2 = FLD_CLK2_EN_ALL;
    SysInit(&SysCtrl);
    SysClockInit(SYS_CLK_HS_DIV, 12);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(1000);
}


unsigned char PlainText[16] = {0x00, 0x11, 0x22, 0x33, 
                               0x44, 0x55, 0x66, 0x77,
                               0x88, 0x99, 0xaa, 0xbb, 
                               0xcc, 0xdd, 0xee, 0xff};
unsigned char Key[16] = {0x00, 0x01, 0x02, 0x03,
                         0x04, 0x05, 0x06, 0x07,
                         0x08, 0x09, 0x0a, 0x0b,
                         0x0c, 0x0d, 0x0e, 0x0f};
unsigned char EncryptResult[16] = {};

unsigned char CipherText[16] = {0x69, 0xc4, 0xe0, 0xd8, 
                                0x6a, 0x7b, 0x04, 0x30,
                                0xd8, 0xcd, 0xb7, 0x80,
                                0x70, 0xb4, 0xc5, 0x5a};
unsigned char DecryptResult[16] = {};

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    unsigned int tmp = 0;
    unsigned char *p = 0;

    LogMsg("aes demo start...\n", NULL, 0);

    AES_Encrypt(Key, PlainText, EncryptResult);
    LogMsg("aes encrypt result: \n", EncryptResult, 16);

    AES_Decrypt(Key, CipherText, DecryptResult);
    LogMsg("aes decrypt result: \n", DecryptResult, 16);

    while(1);
}




