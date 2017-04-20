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
    SysClockInit(SYS_CLK_HS_DIV, 6);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(1000);
}

#define FLASH_TEST_ADDR   (25*4*1024)
#define FLASH_PAGE_SIZE   256 
#define LED2_PIN         GPIOD_GP6     

static unsigned char write_buf[1024];
static unsigned char read_buf[1024];

void main(void)
{
    int len = 0;
    int page_num = 0;
    int offset = 0;

    SYS_Init();

    //led pin initialization
    GPIO_SetGPIOEnable(LED2_PIN, Bit_SET);
    GPIO_SetOutputEnable(LED2_PIN, Bit_SET);
    GPIO_SetBit(LED2_PIN);

    //initialize the buf 
    int i = 0;
    for (i = 0; i < sizeof(write_buf); ++i) {
        write_buf[i] = i % 256;
    }

    //write the buf to flash
    FLASH_SectorErase(FLASH_TEST_ADDR);
    len = sizeof(write_buf);
    page_num = len / FLASH_PAGE_SIZE;
    offset = 0;
    while (page_num--) {
        FLASH_PageWrite(FLASH_TEST_ADDR + offset, FLASH_PAGE_SIZE, write_buf + offset);
        offset += FLASH_PAGE_SIZE;
        len -= FLASH_PAGE_SIZE;
    }
    if (len) {
        FLASH_PageWrite(FLASH_TEST_ADDR + offset, len, write_buf + offset);
    }
    
    //load the flash to read_buf
    memset(read_buf, 0, sizeof(read_buf));
    len = sizeof(read_buf);
    page_num = len / FLASH_PAGE_SIZE;
    offset = 0;
    while (page_num--) {
        FLASH_PageRead(FLASH_TEST_ADDR + offset, FLASH_PAGE_SIZE, read_buf + offset);
        offset += FLASH_PAGE_SIZE;
        len -= FLASH_PAGE_SIZE;
    }
    if (len) {
        FLASH_PageRead(FLASH_TEST_ADDR + offset, len, read_buf + offset);
    }
    
    if (0 == memcmp(write_buf, read_buf, sizeof(read_buf))) {
        while (1) {
            GPIO_ResetBit(LED2_PIN);
            WaitMs(200);
            GPIO_SetBit(LED2_PIN);
            WaitMs(200);
        }
    }
    else {
        while(1);
    }
}




