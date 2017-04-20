#include "../../drivers.h"
#include "../../common.h"

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4
#define NOTIFY_PIN       GPIOD_GP3
#define LED2_PIN         GPIOD_GP6
#define LED4_PIN         GPIOD_GP5
#define LED3_PIN         GPIOD_GP7

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
    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(300);
}

//rf related
volatile unsigned char rf_rx_buf[64] __attribute__ ((aligned (4))) = {0x00};
volatile unsigned char rf_rx_irq_occur = 0;
static unsigned char tx_buf[128] __attribute__ ((aligned (4))) = {0x00};

_attribute_ram_code_ static void build_test_beacon(unsigned char *pBuf)
{
    unsigned char *p = &pBuf[5];
    int len = 0;
    static unsigned char tst_dsn = 0; 

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x18;
    *p++ = ++tst_dsn; //dsn
    *p++ = 0xaa; //dest PANID 
    *p++ = 0xbb;
    *p++ = 0xff; //dest address
    *p++ = 0xff;
    *p++ = 0x33;
    *p++ = 0x55;
    *p++ = 0x66;
    *p++ = 0x77;
    *p++ = 0x88;
    *p++ = 0xcc;
    *p++ = 0xdd;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    //config rf
    RF_RxBufferSet(rf_rx_buf, 64, 0);
    RF_TrxStateSet(RF_MODE_AUTO, 70); //frequency 2470
    WaitMs(10);

    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //led pin initialization
    GPIO_SetGPIOEnable(LED2_PIN|LED4_PIN, Bit_SET);
    GPIO_SetOutputEnable(LED2_PIN|LED4_PIN, Bit_SET);
    GPIO_SetBit(LED2_PIN|LED4_PIN);

    int i = 0;
    while (1) {
        // tx_buf[0] = 4;
        // tx_buf[1] = 0;
        // tx_buf[2] = 0;
        // tx_buf[3] = 0;
        // tx_buf[4] = 5;
        // tx_buf[5] = 0x02;
        // tx_buf[6] = 0x00;
        // tx_buf[7] = 10; //dsn
        RF_StartStx(tx_buf, ClockTime() + 30*TickPerUs);
        build_test_beacon(tx_buf);

        GPIO_ResetBit(LED2_PIN);
        WaitMs(100);
        GPIO_SetBit(LED2_PIN);
        WaitMs(100);
        GPIO_ResetBit(LED2_PIN);
        WaitMs(100);

        WaitMs(1600);
    }
}




