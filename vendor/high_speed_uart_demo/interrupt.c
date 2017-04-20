#include "../../drivers.h"
#include "../../common.h"

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4

extern volatile unsigned char rx_irq_occur;
extern volatile unsigned char tx_irq_occur;
extern volatile unsigned char gpio_irq_occur;

_attribute_ram_code_ void irq_handler(void)
{
    unsigned int IrqSrc = IRQ_SrcGet();

    if (IrqSrc & FLD_IRQ_GPIO_EN) {
        if (0 == GPIO_ReadInputBit(GPIOD_GP2)) {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(GPIOD_GP2)) {
                while(0 == GPIO_ReadInputBit(GPIOD_GP2));
                gpio_irq_occur = 1;
            }
        } 
    }
    
    // if (IrqSrc & FLD_IRQ_DMA_EN) {
        unsigned char UartIrqSrc = IRQ_UartIrqSrcGet();
        if (UartIrqSrc & FLD_UART_IRQ_RX) {
            rx_irq_occur = 1;
        }
        if (UartIrqSrc & FLD_UART_IRQ_TX) {
            tx_irq_occur = 1;
        }
        IRQ_UartIrqSrcClr();

    IRQ_SrcClr();
}



