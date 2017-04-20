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

#define I2C_SLAVE_ADDR           0xD0
#define REG_ADDR_DEV_ID          0X0F
#define CTRL_REG1                0x20
#define CTRL_REG2                0x21

void main(void)
{
    SYS_Init();

    // I2C_pin_select(I2C_PIN_GPIOB); //slect PB6/PB7 as SDA/SCL pin
    I2C_PinSelect(I2C_PIN_GPIOC); //slect PC0/PC1 as SDA/SCL pin
    I2C_Init(I2C_SLAVE_ADDR, 64); //i2c clock = sys clock(16M)/(2*32) = 128k

    LogMsg("i2c demo start...\n", NULL, 0);

    unsigned char data = 0x35;
    unsigned char read_buf[2] = {0, 0};
    unsigned short addr = 0;
    while (1) {
        //write CTRL_REG2
        addr = CTRL_REG2;
        I2C_WriteByte(&addr, 1, data);
        data++;

        //read CTRL_REG2
        read_buf[0] = I2C_ReadByte(&addr, 1);
        LogMsg("data:\n", read_buf, 1);

        WaitMs(1000);
    }
}




