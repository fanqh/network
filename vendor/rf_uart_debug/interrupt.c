#include "../../drivers.h"
#include "../../common.h"

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4

extern volatile unsigned char rf_rx_irq_occur;
extern volatile unsigned char rx_irq_occur;
extern volatile unsigned char tx_irq_occur;
extern volatile unsigned char rf_rx_buf[64];

#define RF_PACKET_LENGTH_OK(p)          (p[0] == p[12]+13)
#define RF_PACKET_CRC_OK(p)             ((p[p[0]+3] & 0x51) == 0x10)

_attribute_ram_code_ void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();
    u8 UartIrqSrc = IRQ_UartIrqSrcGet();

    /* clear the interrupt flag */
    if (RfIrqSrc & FLD_RF_IRQ_RX) {
        if ((rf_rx_buf[0] == 0) || (!RF_PACKET_CRC_OK(rf_rx_buf)) || (!RF_PACKET_LENGTH_OK(rf_rx_buf))) {
            return;
        }
        //receive a valid packet
        else {
            rf_rx_irq_occur = 1;
        }
    }
    IRQ_RfIrqSrcClr();
    
    if (UartIrqSrc & FLD_UART_IRQ_RX) {
        rx_irq_occur = 1;
    }
    if (UartIrqSrc & FLD_UART_IRQ_TX) {
        tx_irq_occur = 1;
    }
    IRQ_UartIrqSrcClr();

    IRQ_SrcClr();
}



