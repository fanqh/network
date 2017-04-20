#include "irq.h"

unsigned char IRQ_Enable(void)
{
    unsigned char r = REG_IRQ_EN;      // don't worry,  the compiler will optimize the return value if not used
    REG_IRQ_EN = 1;
    return r;
}

unsigned char IRQ_Disable(void)
{
    unsigned char r = REG_IRQ_EN;      // don't worry,  the compiler will optimize the return value if not used
    REG_IRQ_EN = 0;
    return r;
}

void IRQ_Restore(unsigned char Enable)
{
    REG_IRQ_EN = Enable;
}

unsigned int IRQ_MaskGet(void)
{
    return REG_IRQ_MASK;
}

void IRQ_MaskSet(unsigned int Mask)
{
    SET_BIT_FLD(REG_IRQ_MASK, Mask);
}

void IRQ_MaskClr(unsigned int Mask)
{
    CLR_BIT_FLD(REG_IRQ_MASK, Mask);
}

unsigned int IRQ_SrcGet(void)
{
    return REG_IRQ_SRC;
}

void IRQ_SrcClr(void)
{
    REG_IRQ_SRC = 0xffffffff;
}

void IRQ_EnableType(unsigned int TypeMask)
{
    IRQ_MaskSet(TypeMask);
}

void IRQ_DisableType(unsigned int TypeMask)
{
    IRQ_MaskClr(TypeMask);
}

//RF module irq
#define REG_RF_IRQ_MASK         REG_ADDR16(0xf1c)
#define REG_RF_IRQ_STATUS       REG_ADDR16(0xf20)

void IRQ_RfIrqEnable(unsigned int RfIrqMask)
{
    REG_RF_IRQ_MASK |= RfIrqMask;
}

void IRQ_RfIrqDisable(unsigned int RfIrqMask)
{
    REG_RF_IRQ_MASK &= (~RfIrqMask);
}

unsigned short IRQ_RfIrqSrcGet(void)
{
    return REG_RF_IRQ_STATUS;
}

void IRQ_RfIrqSrcClr()
{
    REG_RF_IRQ_STATUS = 0xffff;
}

//SPI module irq
#define REG_SPI_IRQ_STATUS       REG_ADDR8(0x21)
#define REG_SPI_IRQ_CLR_STATUS   REG_ADDR8(0x22)

unsigned char IRQ_SpiIrqSrcGet(void)
{
    return REG_SPI_IRQ_STATUS;
}

void IRQ_SpiIrqSrcClr(void)
{
    REG_SPI_IRQ_CLR_STATUS = FLD_SPI_IRQ_HOST_CMD | FLD_SPI_IRQ_HOST_RD_TAG;
}

//UART module irq
#define REG_UART_IRQ_MASK         REG_ADDR8(0x521)
#define REG_UART_IRQ_STATUS       REG_ADDR8(0x526)
#define REG_UART_IRQ_CLR_STATUS   REG_ADDR8(0x526)

void IRQ_UartIrqEnable(unsigned int RfIrqMask)
{
    if (RfIrqMask) {
        if (RfIrqMask & FLD_UART_IRQ_RX) {
            REG_UART_IRQ_MASK |= FLD_UART_IRQ_RX; //open dma1 interrupt mask
        }

        if (RfIrqMask & FLD_UART_IRQ_TX) {
            REG_UART_IRQ_MASK |= FLD_UART_IRQ_TX; //open dma1 interrupt mask
        }
    }
}

void IRQ_UartIrqDisable(unsigned int RfIrqMask)
{
    if (RfIrqMask & FLD_UART_IRQ_RX) {
        REG_UART_IRQ_MASK &= ~FLD_UART_IRQ_RX; //close dma1 interrupt mask
    }

    if (RfIrqMask & FLD_UART_IRQ_TX) {
        REG_UART_IRQ_MASK &= ~FLD_UART_IRQ_TX; //close dma1 interrupt mask
    }
}

unsigned char IRQ_UartIrqSrcGet(void)
{
    return REG_UART_IRQ_STATUS;
}

void IRQ_UartIrqSrcClr(void)
{
    REG_UART_IRQ_CLR_STATUS |= (FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);  // set to clear
}