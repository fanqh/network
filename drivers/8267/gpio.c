#include "gpio.h"

/** define BIT operations */ 
#define BM_SET(addr,bit)                ( *(addr) |= (bit) )
#define BM_CLR(addr,bit)                ( *(addr) &= ~(bit) )
#define BM_IS_SET(addr,bit)             ( *(addr) & (bit) )

/** define GPIO related registers */ 
#define REG_BASE                0X800000
#define REG_ADDR_8(addr)        (volatile unsigned char  *)(REG_BASE+addr)
#define REG_GPIO_IN(i)          REG_ADDR_8(0x580+((i>>8)<<3))
#define REG_GPIO_IE(i)          REG_ADDR_8(0x581+((i>>8)<<3))
#define REG_GPIO_OEN(i)         REG_ADDR_8(0x582+((i>>8)<<3))
#define REG_GPIO_OUT(i)         REG_ADDR_8(0x583+((i>>8)<<3))
#define REG_GPIO_POL(i)         REG_ADDR_8(0x584+((i>>8)<<3))
#define REG_GPIO_DS(i)          REG_ADDR_8(0x585+((i>>8)<<3))
#define REG_GPIO_GPIO_FUNC(i)   REG_ADDR_8(0x586+((i>>8)<<3))
#define REG_GPIO_IRQ_EN(i)      REG_ADDR_8(0x587+((i>>8)<<3))
#define REG_GPIO_IRQ_SRC(i)     REG_ADDR_8(0x5e0+(i>>8))
#define REG_GPIO_2RISC0(i)      REG_ADDR_8(0x5b8+(i>>8))
#define REG_GPIO_2RISC1(i)      REG_ADDR_8(0x5c0+(i>>8))
#define REG_GPIO_2RISC2(i)      REG_ADDR_8(0x5c8+(i>>8))


/**
 * @brief This function set the GPIO function of a pin.
 * @param[in]   pin the pin needs to set the GPIO function
 * @param[in]   value enable or disable the pin's GPIO function(0: disable, 1: enable)
 * @return none
 */
void GPIO_SetGPIOEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value)
{
    unsigned char bit = pin & 0xff;
    if (value) {
        BM_SET(REG_GPIO_GPIO_FUNC(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_GPIO_FUNC(pin), bit);
    }
}

/**
 * @brief This function set the output function of a pin.
 * @param[in]   pin the pin needs to set the output function
 * @param[in]   value enable or disable the pin's output function(0: disable, 1: enable)
 * @return none
 */
void GPIO_SetOutputEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value)
{
    unsigned char bit = pin & 0xff;
    if (!value) {
        BM_SET(REG_GPIO_OEN(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_OEN(pin), bit);
    }
}

/**
 * @brief This function set the input function of a pin.
 * @param[in]   pin the pin needs to set the input function
 * @param[in]   value enable or disable the pin's input function(0: disable, 1: enable)
 * @return none
 */
void GPIO_SetInputEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value)
{
    unsigned char bit = pin & 0xff;
    if (value) {
        BM_SET(REG_GPIO_IE(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_IE(pin), bit);
    }
}

/**
 * @brief This function determines whether the output function of a pin is enabled.
 * @param[in]   pin the pin needs to determine whether its output function is enabled.
 * @return 1: the pin's output function is enabled 0: the pin's output function is disabled
 */
int GPIO_IsOutputEnable(GPIO_PinTypeDef pin)
{
    return !BM_IS_SET(REG_GPIO_OEN(pin), pin & 0xff);
}

/**
 * @brief This function determines whether the input function of a pin is enabled.
 * @param[in]   pin the pin needs to determine whether its input function is enabled.
 * @return 1: the pin's input function is enabled 0: the pin's input function is disabled
 */
int GPIO_IsInputEnable(GPIO_PinTypeDef pin)
{
    return BM_IS_SET(REG_GPIO_IE(pin), pin & 0xff);
}

/**
 * @brief This function set the pin's driving strength.
 * @param[in]   pin the pin needs to set the driving strength
 * @param[in]   value the level of driving strength(1: strong 0: poor)
 * @return none
 */
void GPIO_SetDataStrength(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value)
{
    unsigned char bit = pin & 0xff;
    if (value) {
        BM_SET(REG_GPIO_DS(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_DS(pin), bit);
    }
}

/**
 * @brief This function set the pin's output level as high.
 * @param[in]   pin the pin needs to set its output level
 * @return none
 */
void GPIO_SetBit(GPIO_PinTypeDef pin)
{
    BM_SET(REG_GPIO_OUT(pin), (unsigned char)(pin & 0xff));
}

/**
 * @brief This function set the pin's output level as low.
 * @param[in]   pin the pin needs to set its output level
 * @return none
 */
void GPIO_ResetBit(GPIO_PinTypeDef pin)
{
    BM_CLR(REG_GPIO_OUT(pin), (unsigned char)(pin & 0xff));
}

/**
 * @brief This function set the pin's output level.
 * @param[in]   pin the pin needs to set its output level
 * @param[in]   value value of the output level(1: high 0: low)
 * @return none
 */
void GPIO_WriteBit(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value)
{
    unsigned char bit = pin & 0xff;
    if (value) {
        BM_SET(REG_GPIO_OUT(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_OUT(pin), bit);
    }
}

/**
 * @brief This function read the pin's input level.
 * @param[in]   pin the pin needs to read its input level
 * @return the pin's input level(1: high 0: low)
 */
unsigned long GPIO_ReadInputBit(GPIO_PinTypeDef pin)
{
    return BM_IS_SET(REG_GPIO_IN(pin), pin & 0xff);
}

/**
 * @brief This function read the pin's output level.
 * @param[in]   pin the pin needs to read its output level
 * @return the pin's output level(1: high 0: low)
 */
unsigned long GPIO_ReadOutputBit(GPIO_PinTypeDef pin)
{
    return BM_IS_SET(REG_GPIO_OUT(pin), pin & 0xff);
}

/**
 * @brief This function read all the pins' input level.
 * @param[out]   p the buffer used to store all the pins' input level
 * @return none
 */
void GPIO_ReadAll(unsigned char *p)
{
    p[0] = *REG_ADDR_8(0x580);
    p[1] = *REG_ADDR_8(0x588);
    p[2] = *REG_ADDR_8(0x590);
    p[3] = *REG_ADDR_8(0x598);
    p[4] = *REG_ADDR_8(0x5a0);
}

/**
 * @brief This function read a pin's input level from the buffer.
 * @param[in]   pin the pin needs to read its input level from the buffer
 * @param[in]   buf the buf from which to read the pin's input level
 * @return none
 */
unsigned long GPIO_ReadCache(GPIO_PinTypeDef pin, unsigned char *p)
{
    return p[pin>>8] & (pin & 0xff);
}

/**
 * @brief This function set a pin's pull-up/down resistor.
 * @param[in]   pin the pin needs to set its pull-up/down resistor
 * @param[in]   type the type of the pull-up/down resistor
 * @return none
 */
void GPIO_PullSet(GPIO_PinTypeDef pin, GPIO_PullTypeDef type)
{
    unsigned char bit = pin & 0xff;
    unsigned char group = pin >> 8;
    unsigned char areg;
    unsigned char offset;
    unsigned char val;
    unsigned char i;

    /***enable pull-up or pull-down***/
    //specially for E2/E3 pin
    if (group == 4) {
        if (bit & GPIOE_GP2) {
            val = ReadAnalogReg(0x08);
            val &= 0xcf;
            val |= (type<<4);
            WriteAnalogReg(0x08, val);   
        }
        if (bit & GPIOE_GP3) {
            val = ReadAnalogReg(0x08);
            val &= 0x3f;
            val |= (type<<6);
            WriteAnalogReg(0x08, val);   
        }
    }
    //for other pins
    for (i=0; i<8; i++) {
        if ((1<<i) & bit) {
            areg = 0x0a + ((group*8+i)*2+4)/8;
            offset = ((group*8+i)*2+4) % 8;
            val = ReadAnalogReg(areg);
            val &= ~(0x03<<offset);
            val |= (type<<offset);
            WriteAnalogReg(areg, val);   
        }
    }
}

/**
 * @brief This function enables a pin's IRQ function.
 * @param[in]   pin the pin needs to enables its IRQ function.
 * @return none
 */
void GPIO_EnableInterrupt(GPIO_PinTypeDef pin)
{
    unsigned char bit = pin & 0xff;
    BM_SET(REG_GPIO_IRQ_EN(pin), bit);
    BM_SET(REG_ADDR_8(0x5b5), BIT(3));
}

/**
 * @brief This function set a pin's polarity that trig its IRQ.
 * @param[in]   pin the pin needs to set its edge polarity that trig its IRQ
 * @param[in]   falling value of the edge polarity(1: falling edge 0: rising edge)
 * @return none
 */
void GPIO_SetInterruptPolarity(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling)
{
    unsigned char bit = pin & 0xff;
    if (falling) {
        BM_SET(REG_GPIO_POL(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_POL(pin), bit);
    }
}

/**
 * @brief This function set a pin's IRQ.
 * @param[in]   pin the pin needs to enable its IRQ
 * @param[in]   falling value of the edge polarity(1: falling edge 0: rising edge)
 * @return none
 */
void GPIO_SetInterrupt(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling)
{
    unsigned char bit = pin & 0xff;
    BM_SET(REG_GPIO_IRQ_EN(pin), bit);
    BM_SET(REG_ADDR_8(0x5b5), BIT(3));
    if (falling) {
        BM_SET(REG_GPIO_POL(pin), bit);
    }
    else {
        BM_CLR(REG_GPIO_POL(pin), bit);
    }
}

/**
 * @brief This function disables a pin's IRQ function.
 * @param[in]   pin the pin needs to disables its IRQ function.
 * @return none
 */
void GPIO_ClearInterrupt(GPIO_PinTypeDef pin)
{
    unsigned char bit = pin & 0xff;
    BM_CLR(REG_GPIO_IRQ_EN(pin), bit);
}

#if 0
/**
 * @brief This function determines whether a pin's IRQ occurs.
 * @param[in]   pin the pin needs to determine whether its IRQ occurs.
 * @return none
 */
int GPIO_IsInterruptOccur(GPIO_PinTypeDef pin)
{
    return BM_IS_SET(REG_GPIO_IRQ_SRC(pin), pin & 0xff);
}
#endif /* #if 0 */