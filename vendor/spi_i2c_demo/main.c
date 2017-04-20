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

#if 1
#define REG_ADDR_DEV_ID    0X0F
#define CTRL_REG1          0x20
#define CTRL_REG2          0x21
#define CS_PIN_SENSOR1     GPIOA_GP5

#define I2C_SLAVE_ADDR           0xD0
#define REG_ADDR_DEV_ID          0X0F

void main(void)
{
	SYS_Init();

    LogMsg("spi and i2c demo start...\n", NULL, 0);

    unsigned char cmd_buf[1] = {0};
    unsigned short test_cnt = 0x1122;
    unsigned char read_buf[2] = {0, 0};

    unsigned char data = 0x35;
    unsigned char read_buf2[2] = {0, 0};
    unsigned short addr = 0;

    while (1) {
        //config SPI master
        SPI_PinSelect(SPI_PIN_GPIOA);
        SPI_CSPinSelect(CS_PIN_SENSOR1);
        SPI_Init(0x25, SPI_MODE0);
    
        // //read device ID (0xd3)
        // cmd_buf[0] = REG_ADDR_DEV_ID & 0x3f; //bit0-bit5 address
        // cmd_buf[0] &= (~BIT(6)); //bit6 0: single byte 1: multiple byte
        // cmd_buf[0] |= BIT(7); //bit7 0: write 1: read
        // SPI_Read(cmd_buf, 1, read_buf, 1, CS_PIN_SENSOR1);
        // LogMsg("spi dev_id:\n", read_buf, 1);

        // //read CTRL_REG1(default: 0x07)
        // cmd_buf[0] = CTRL_REG1 & 0x3f; //bit0-bit5 address
        // cmd_buf[0] &= (~BIT(6)); //bit6 0: single byte 1: multiple byte
        // cmd_buf[0] |= BIT(7); //bit7 0: write 1: read
        // SPI_Read(cmd_buf, 1, read_buf, 1, CS_PIN_SENSOR1);
        // LogMsg("spi CTRL_REG1:\n", read_buf, 1);

        //write CTRL_REG2|CTRL_REG1 (2 bytes)
        cmd_buf[0] = CTRL_REG1 & 0x3f; //bit0-bit5 address
        cmd_buf[0] |= BIT(6); //bit6 0: single byte 1: multiple byte
        cmd_buf[0] &= (~BIT(7)); //bit7 0: write 1: read
        SPI_Write(cmd_buf, 1, &test_cnt, 2, CS_PIN_SENSOR1);
        test_cnt++;

        //read CTRL_REG2|CTRL_REG1 (2 bytes)
        cmd_buf[0] = CTRL_REG1 & 0x3f; //bit0-bit5 address
        cmd_buf[0] |= BIT(6); //bit6 0: single byte 1: multiple byte
        cmd_buf[0] |= BIT(7); //bit7 0: write 1: read
        SPI_Read(cmd_buf, 1, read_buf, 2, CS_PIN_SENSOR1);
        LogMsg("spi read_test_cnt:\n", read_buf, 2);
   
        //config I2C master
        I2C_PinSelect(I2C_PIN_GPIOC); //slect PC0/PC1 as SDA/SCL pin
        I2C_Init(I2C_SLAVE_ADDR, 64); //i2c clock = sys clock(16M)/(2*32) = 128k

        //write CTRL_REG2
        addr = CTRL_REG2;
        I2C_WriteByte(&addr, 1, data);
        data++;

        //read CTRL_REG2
        read_buf2[0] = I2C_ReadByte(&addr, 1);
        LogMsg("i2c read data:\n", read_buf2, 1);
 
        WaitMs(1000);
    }
}
#endif /* if 0 */

#if 0



static unsigned short slave_buf_addr;
static unsigned char cmd[8] = {0};
static unsigned int test_cnt = 0x00000001;
static unsigned char SPI_Write_buffer[128] = {0};

void main(void)
{
    sys_init();

    //config SPI master
    SPI_pin_select(SPI_PIN_GPIOA);
    SPI_CS_pin_select(GPIOA_GP5);
    SPI_init(0x25, SPI_MODE0);

    //config I2C master
    I2C_pin_select(I2C_PIN_GPIOB); //slect PB6/PB7 as SDA/SCL pin
    I2C_init(I2C_SLAVE_ADDR, 64); //i2c clock = sys clock(16M)/(2*32) = 128k

    WaitMs(3000);

    LogMsg("spi and i2c master demo start...\n", NULL, 0);

    //read spi buffer address of slave which is stored in slave's 0x808004 and 0x808005 register
    cmd[0] = 0x80; //address high
    cmd[1] = 0x04; //address low
    cmd[2] = 0x80; //read operation command
    SPI_Read(cmd, 3, &slave_buf_addr, 2, GPIOA_GP5);
    LogMsg("spi buffer address of slave is:\n", &slave_buf_addr, 2);

    while (1) {
        //read device ID (0xd3)
        SPI_Write_buffer[0] = sizeof(test_cnt); //bit0-bit5 address
        memcpy(SPI_Write_buffer+1, &test_cnt, sizeof(test_cnt));
        cmd[0] = slave_buf_addr >> 8; //address high
        cmd[1] = slave_buf_addr & 0xff; //address high
        cmd[2] = 0x00; //write operation command
        SPI_Write(cmd, 3, SPI_Write_buffer, 1+SPI_Write_buffer[0], GPIOA_GP5);
        LogMsg("write test_cnt to slave...\n", NULL, 0);
        test_cnt++;

        WaitMs(1000);
    }
}

void main(void)
{
    sys_init();

    I2C_PinSelect(I2C_PIN_GPIOB); //slect PB6/PB7 as SDA/SCL pin
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

#endif /* if 1 */




