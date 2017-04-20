#include "../../drivers.h"
#include "../../common.h"

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4
#define NOTIFY_PIN       GPIOD_GP3
#define LED2_PIN         GPIOD_GP6
#define LED4_PIN         GPIOD_GP5
#define LED3_PIN         GPIOD_GP7

unsigned long firmwareVersion;

static void prepare_to_sleep(void)
{
    //This function disable all unecessary GPIOs avoiding current leakage
    unsigned char mask = WAKE_UP_PIN_1 | WAKE_UP_PIN_2;
    //disable OEN
    GPIO_SetOutputEnable(GPIOA_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOB_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOC_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOD_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOE_ALL, Bit_RESET);

    //disable dataO
    GPIO_ResetBit(GPIOA_ALL);
    GPIO_ResetBit(GPIOB_ALL);
    GPIO_ResetBit(GPIOC_ALL);
    GPIO_ResetBit(GPIOD_ALL);
    GPIO_ResetBit(GPIOE_ALL);

    //disable input_en
    GPIO_SetInputEnable(GPIOA_ALL, Bit_RESET);
    // GPIO_SetInputEnable(GPIOB_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOC_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOD_ALL^(mask), Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP0, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP1, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP2, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP3, Bit_RESET);
    //disable the input of DM/DP pins for eliminating the leakage current
    USB_DpPullUpEn(0);
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
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(1000);
}

static volatile unsigned char rxBuf[128] = {0};
static unsigned char txBuf[128] = {0};
static unsigned int test_cnt = 0;
volatile unsigned char rx_irq_occur = 0;
volatile unsigned char tx_irq_occur = 0;
volatile unsigned char gpio_irq_occur = 0;

static volatile unsigned int rx_len = 0;
void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    LogMsg("UART Demo start...", NULL, 0);

    //led pin initialization
    GPIO_SetGPIOEnable(LED2_PIN, Bit_SET);
    GPIO_SetOutputEnable(LED2_PIN, Bit_SET);
    GPIO_SetBit(LED2_PIN);
    WaitMs(300);
    GPIO_ResetBit(LED2_PIN);
    WaitMs(300);
    GPIO_SetBit(LED2_PIN);
    WaitMs(300);
    GPIO_ResetBit(LED2_PIN);
    WaitMs(300);

    //config wakeup pin
    GPIO_SetGPIOEnable(WAKE_UP_PIN_1|WAKE_UP_PIN_2, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(WAKE_UP_PIN_1|WAKE_UP_PIN_2, Bit_SET);   //enable input
    GPIO_PullSet(WAKE_UP_PIN_1|WAKE_UP_PIN_2, PULL_UP_1M);       //pull up
    GPIO_SetInterrupt(WAKE_UP_PIN_1, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
    PM_GPIOSet(WAKE_UP_PIN_1|WAKE_UP_PIN_2, 0, 1);

    //config UART module
    GPIO_PullSet(GPIOB_GP2|GPIOB_GP3, PULL_UP_1M);
    UART_RecBuffInit(rxBuf, sizeof(rxBuf));
    UART_GPIO_CFG_PB2_PB3();
    UART_Init(921600, PARITY_NONE, STOP_BIT_ONE);
    IRQ_UartIrqEnable(FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);
    IRQ_EnableType(FLD_IRQ_DMA_EN);
    IRQ_Enable();

    while (1) {
        //enter suspend
        prepare_to_sleep();
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_DIG_GPIO, 0);

        if (gpio_irq_occur) {
            gpio_irq_occur = 0;
            GPIO_SetGPIOEnable(LED4_PIN, Bit_SET);
            GPIO_SetOutputEnable(LED4_PIN, Bit_SET);
            GPIO_SetBit(LED4_PIN);
            WaitMs(300);
            GPIO_ResetBit(LED4_PIN);
            WaitMs(300);
            GPIO_SetBit(LED4_PIN);
            WaitMs(300);
            GPIO_ResetBit(LED4_PIN);
            WaitMs(300);

            //wakeup the peer first
            GPIO_SetGPIOEnable(NOTIFY_PIN, Bit_SET);
            GPIO_ResetBit(NOTIFY_PIN);
            GPIO_SetOutputEnable(NOTIFY_PIN, Bit_SET);
            WaitUs(1);
            GPIO_SetOutputEnable(NOTIFY_PIN, Bit_RESET);
            WaitMs(5);

            //send data to peer
            txBuf[0] = strlen("Telink-UART-Demo");
            test_cnt++;
            memcpy(&txBuf[4], "Telink-UART-Demo", txBuf[0]);
            while (!UART_Send(txBuf));
            while(!tx_irq_occur);
            tx_irq_occur = 0;
        }
        else {
            //wait for incoming data via UART
            while (1) {
                if (rx_irq_occur) {
                    rx_irq_occur = 0;

                    test_cnt++;
                    rx_len = (rxBuf[3]<<24) + (rxBuf[2]<<16) + (rxBuf[1]<<8) + rxBuf[0];
                    rxBuf[4+rx_len] = 0;
                    if (0 == strcmp(&rxBuf[4], "Telink-UART-Demo")) {
                        GPIO_SetGPIOEnable(LED3_PIN, Bit_SET);
                        GPIO_SetOutputEnable(LED3_PIN, Bit_SET);
                        GPIO_SetBit(LED3_PIN);
                        WaitMs(300);
                        GPIO_ResetBit(LED3_PIN);
                        WaitMs(300);
                        GPIO_SetBit(LED3_PIN);
                        WaitMs(300);
                        GPIO_ResetBit(LED3_PIN);
                        WaitMs(300);
                    }
                    rxBuf[0] = 0;
                    break;
                }
            }
        }

        //This delay ensure that the mcu will stay active
        //at least 2ms before entering suspend again
        WaitMs(3); 
    }
}




