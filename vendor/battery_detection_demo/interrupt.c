#include "../../common.h"

extern volatile unsigned char rx_irq_occur;
extern volatile unsigned char tx_irq_occur;

_attribute_ram_code_ void irq_handler(void)
{
    unsigned int IrqSrc = IRQ_SrcGet();
    
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


