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

static unsigned char spi_buf[512];
unsigned char spi_read_flag = 0;
unsigned char spi_write_flag = 0;
void main(void)
{
	SYS_Init();

    //set address of spi buffer to general-purpose register 0x808004,0x808005
    write_reg16(0x808004, spi_buf);
    WaitMs(1000);

    //configuration for SPI slave
    SPI_SlavePinSelect(SPI_PIN_GPIOA);
    SPI_SlaveInit(0x25, SPI_MODE0);
    //spi irq enable
    irq_init(FLD_IRQ_HOST_CMD_EN);
    irq_enable();

    LogMsg("spi slave demo start...\n", NULL, 0);

    
    while (1) {
        if (spi_read_flag) {
            spi_read_flag = 0;
            LogMsg("spi master reads bytes from slave...\n", NULL, 0);
        }

        if (spi_write_flag) {
            spi_write_flag = 0;
            LogMsg("spi master writes bytes to slave...\n", spi_buf+1, spi_buf[0]);
        }
    }
}




