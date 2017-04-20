#include "../../drivers.h"
#include "../../common.h"
#include "usb_desc.h"

unsigned long  firmwareVersion;

volatile unsigned char rx_irq_occur = 0;
volatile unsigned char tx_irq_occur = 0;

//define rx buffer
#define RX_BUF_LEN    64 //in bytes
#define RX_BUF_NUM    4
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM];
static unsigned char rx_ptr = 0;

static unsigned char uart_rx_buf[RX_BUF_LEN];
static unsigned char uart_tx_buf[RX_BUF_LEN];

//define rx callback function
static void USBCDC_RxCb(unsigned char *data)
{
    //set next rx buffer
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    USBCDC_RxBufSet(rx_buf + rx_ptr*RX_BUF_LEN);

    GPIO_ResetBit(GPIOD_GP6);
    WaitMs(50);
    GPIO_SetBit(GPIOD_GP6);
    WaitMs(50);
    GPIO_ResetBit(GPIOD_GP6);
    WaitMs(50);

    //process received data
    memcpy(&uart_tx_buf[4], data, strlen(data));
    uart_tx_buf[0] = strlen(data);
    uart_tx_buf[1] = 0;
    uart_tx_buf[2] = 0;
    uart_tx_buf[3] = 0;
    memset(data, 0, RX_BUF_LEN);
    while (!UART_Send(uart_tx_buf));
    while(!tx_irq_occur);
    tx_irq_occur = 0;
}

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
    USB_Init();
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    USBCDC_RxBufSet(rx_buf + rx_ptr*RX_BUF_LEN);
    USBCDC_CBSet(USBCDC_RxCb, NULL);

    //config UART module
    GPIO_PullSet(GPIOB_GP2|GPIOB_GP3, PULL_UP_1M);
    UART_RecBuffInit(uart_rx_buf, sizeof(uart_rx_buf));
    UART_GPIO_CFG_PB2_PB3();
    UART_Init(921600, PARITY_NONE, STOP_BIT_ONE);
    IRQ_UartIrqEnable(FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);
    IRQ_EnableType(FLD_IRQ_DMA_EN);
    IRQ_Enable();

    //led pin initialization
    GPIO_SetGPIOEnable(GPIOD_GP6, Bit_SET);
    GPIO_SetOutputEnable(GPIOD_GP6, Bit_SET);
    GPIO_SetBit(GPIOD_GP6);

    while (1) {
        ev_process_timer();
        USB_IrqHandle();
        if (rx_irq_occur) {
            rx_irq_occur = 0;
            if (USBCDC_IsAvailable()) {
                USBCDC_DataSend(&uart_rx_buf[4], uart_rx_buf[0]);
                memset(uart_rx_buf, 0, sizeof(uart_rx_buf));
            }
            GPIO_ResetBit(GPIOD_GP6);
            WaitMs(100);
            GPIO_SetBit(GPIOD_GP6);
            WaitMs(100);
            GPIO_ResetBit(GPIOD_GP6);
            WaitMs(100);
        }
    }
}
