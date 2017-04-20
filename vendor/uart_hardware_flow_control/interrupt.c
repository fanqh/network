#include "../../drivers.h"
#include "../../common.h"

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4
#define LED2_PIN         GPIOD_GP6
#define LED4_PIN         GPIOD_GP5
#define LED3_PIN         GPIOD_GP7

extern volatile unsigned char rx_irq_occur;
extern volatile unsigned char tx_irq_occur;

extern volatile unsigned char rxBuf[];

_attribute_ram_code_ void irq_handler(void)
{
    unsigned int IrqSrc = IRQ_SrcGet();
    
    unsigned char UartIrqSrc = IRQ_UartIrqSrcGet();
    if (UartIrqSrc & FLD_UART_IRQ_RX) {
        rx_irq_occur = 1;

        rxBuf[4 + rxBuf[0]] = '\0';
        if (strcmp(&rxBuf[4], "uart_demo") == 0) {
            GPIO_ResetBit(LED4_PIN);
            WaitMs(60);
            GPIO_SetBit(LED4_PIN);
            WaitMs(60);
            GPIO_ResetBit(LED4_PIN);
            WaitMs(60);
        }
    }
    if (UartIrqSrc & FLD_UART_IRQ_TX) {
        tx_irq_occur = 1;
    }
    IRQ_UartIrqSrcClr();

    IRQ_SrcClr();
}



