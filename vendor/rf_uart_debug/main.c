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

static volatile unsigned char rxBuf[128] = {0};
static unsigned char txBuf[128] = {0};
static unsigned int test_cnt = 0;
volatile unsigned char rx_irq_occur = 0;
volatile unsigned char tx_irq_occur = 0;
volatile unsigned char gpio_irq_occur = 0;
static volatile unsigned int rx_len = 0;

//rf related
volatile unsigned char rf_rx_buf[64] __attribute__ ((aligned (4))) = {0x00};
volatile unsigned char rf_rx_irq_occur = 0;

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    //config rf
    RF_RxBufferSet(rf_rx_buf, 64, 0);
    RF_TrxStateSet(RF_MODE_RX, 70); //frequency 2470
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX);

    //led pin initialization
    GPIO_SetGPIOEnable(LED2_PIN|LED4_PIN, Bit_SET);
    GPIO_SetOutputEnable(LED2_PIN|LED4_PIN, Bit_SET);
    GPIO_SetBit(LED2_PIN|LED4_PIN);

    //config UART module
    GPIO_PullSet(GPIOB_GP2|GPIOB_GP3, PULL_UP_1M);
    UART_RecBuffInit(rxBuf, sizeof(rxBuf));
    UART_GPIO_CFG_PB2_PB3();
    UART_Init(921600, PARITY_NONE, STOP_BIT_ONE);
    IRQ_UartIrqEnable(FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);
    IRQ_EnableType(FLD_IRQ_DMA_EN);
    IRQ_Enable();

    int i = 0;
    while (1) {
        if (rx_irq_occur) {
            rx_irq_occur = 0;
            test_cnt++;
            txBuf[0] = rxBuf[0];
            txBuf[1] = 0;
            txBuf[2] = 0;
            txBuf[3] = 0;
            
            for (i = 0; i < rxBuf[0]; i++) {
                txBuf[4+i] = rxBuf[4+i];
            }
            while (!UART_Send(txBuf));
            while(!tx_irq_occur);
            tx_irq_occur = 0;

            GPIO_ResetBit(LED2_PIN);
            WaitMs(100);
            GPIO_SetBit(LED2_PIN);
            WaitMs(100);
            GPIO_ResetBit(LED2_PIN);
            WaitMs(100);
        }

        if (rf_rx_irq_occur) {
            rf_rx_irq_occur = 0;
            GPIO_ResetBit(LED4_PIN);
            WaitMs(100);
            GPIO_SetBit(LED4_PIN);
            WaitMs(100);
            GPIO_ResetBit(LED4_PIN);
            WaitMs(100);
        }
    }
}




