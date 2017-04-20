/********************************************************************************************************
 * @file     gpio.h
 *
 * @brief    This file provides set of functions to manage GPIOs
 *
 * @author   jian.zhang@telink-semi.com
 * @date     Oct. 8, 2016
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *           The information contained herein is confidential property of Telink 
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *           Co., Ltd. and the licensee or the terms described here-in. This heading 
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this 
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#ifndef GPIO_H
#define GPIO_H

#include "bsp.h"

typedef enum {
    Bit_RESET = 0,
    Bit_SET,
} GPIO_BitActionTypeDef;

//GPIO types
typedef enum {
    GPIO_GROUPA    = 0x000,
    GPIO_GROUPB    = 0x100,
    GPIO_GROUPC    = 0x200,
    GPIO_GROUPD    = 0x300,
    GPIO_GROUPE    = 0x400,
} GPIO_GroupTypeDef;

/**
 *  @brief  Define pull-up/down types 
 */
typedef enum {
    PULL_NONE      = 0,
    PULL_UP_1M     = 1,
    PULL_UP_10K    = 2,
    PULL_DOWN_100K = 3,
} GPIO_PullTypeDef;


typedef enum {
    GPIOA_GP0  = GPIO_GROUPA | BIT(0), GPIO_PWM0A0=GPIOA_GP0,   GPIO_DMIC_DI=GPIOA_GP0,
    GPIOA_GP1  = GPIO_GROUPA | BIT(1),                          GPIO_DMIC_CK=GPIOA_GP1,
    GPIOA_GP2  = GPIO_GROUPA | BIT(2), GPIO_PWM0NA2=GPIOA_GP2,  GPIO_DOA2=GPIOA_GP2,
    GPIOA_GP3  = GPIO_GROUPA | BIT(3), GPIO_PWM1A3=GPIOA_GP3,   GPIO_DIA3=GPIOA_GP3,
    GPIOA_GP4  = GPIO_GROUPA | BIT(4), GPIO_PWM1NA4=GPIOA_GP4,  GPIO_CKA4=GPIOA_GP4,
    GPIOA_GP5  = GPIO_GROUPA | BIT(5), GPIO_PWM2NA5=GPIOA_GP5,  GPIO_CNA5=GPIOA_GP5,
    GPIOA_GP6  = GPIO_GROUPA | BIT(6),                          GPIO_UTXA6=GPIOA_GP6,
    GPIOA_GP7  = GPIO_GROUPA | BIT(7),                          GPIO_URXA7=GPIOA_GP7, GPIO_SWM=GPIOA_GP7,
    GPIOA_ALL  = GPIO_GROUPA | 0X00ff,  

    GPIOB_GP0  = GPIO_GROUPB | BIT(0), GPIO_PWM2B0=GPIOB_GP0,   GPIO_SWS=GPIOB_GP0,
    GPIOB_GP1  = GPIO_GROUPB | BIT(1), GPIO_PWM2NB1=GPIOB_GP1,
    GPIOB_GP2  = GPIO_GROUPB | BIT(2), GPIO_PWM3B2=GPIOB_GP2,   GPIO_UTXB2=GPIOB_GP2,
    GPIOB_GP3  = GPIO_GROUPB | BIT(3), GPIO_PWM3NB3=GPIOB_GP3,  GPIO_URXB3=GPIOB_GP3,
    GPIOB_GP4  = GPIO_GROUPB | BIT(4), GPIO_PWM4B4=GPIOB_GP4,   GPIO_CNB4=GPIOB_GP4,
    GPIOB_GP5  = GPIO_GROUPB | BIT(5), GPIO_PWM4NB5=GPIOB_GP5,  GPIO_DOB5=GPIOB_GP5,
    GPIOB_GP6  = GPIO_GROUPB | BIT(6), GPIO_PWM5B6=GPIOB_GP6,   GPIO_DIB6=GPIOB_GP6,
    GPIOB_GP7  = GPIO_GROUPB | BIT(7), GPIO_PWM5NB7=GPIOB_GP7,  GPIO_CKB7=GPIOB_GP7,
    GPIOB_ALL  = GPIO_GROUPB | 0x00ff,

    GPIOC_GP0  = GPIO_GROUPC | BIT(0),GPIO_PWM0C0=GPIOC_GP0,    GPIO_I2C_SDA=GPIOC_GP0,
    GPIOC_GP1  = GPIO_GROUPC | BIT(1),GPIO_PWM1C1=GPIOC_GP1,    GPIO_I2C_SCK=GPIOC_GP1,
    GPIOC_GP2  = GPIO_GROUPC | BIT(2),GPIO_PWM2C2=GPIOC_GP2,    GPIO_UTXC2=GPIOC_GP2,
    GPIOC_GP3  = GPIO_GROUPC | BIT(3),GPIO_PWM3C3=GPIOC_GP3,    GPIO_URXC3=GPIOC_GP3,
    GPIOC_GP4  = GPIO_GROUPC | BIT(4),GPIO_PWM4C4=GPIOC_GP4,    GPIO_URTSC4=GPIOC_GP4,
    GPIOC_GP5  = GPIO_GROUPC | BIT(5),GPIO_PWM4C5=GPIOC_GP5,    GPIO_UCTSC5=GPIOC_GP5,
    GPIOC_GP6  = GPIO_GROUPC | BIT(6),
    GPIOC_GP7  = GPIO_GROUPC | BIT(7),
    GPIOC_ALL  = GPIO_GROUPC | 0x00ff,

    GPIOD_GP0  = GPIO_GROUPD | BIT(0),
    GPIOD_GP1  = GPIO_GROUPD | BIT(1),
    GPIOD_GP2  = GPIO_GROUPD | BIT(2),
    GPIOD_GP3  = GPIO_GROUPD | BIT(3),
    GPIOD_GP4  = GPIO_GROUPD | BIT(4),
    GPIOD_GP5  = GPIO_GROUPD | BIT(5),GPIO_PWM0D5=GPIOD_GP5,
    GPIOD_GP6  = GPIO_GROUPD | BIT(6),GPIO_PWM1D6=GPIOD_GP6,
    GPIOD_GP7  = GPIO_GROUPD | BIT(7),GPIO_PWM2D7=GPIOD_GP7,
    GPIOD_ALL  = GPIO_GROUPD | 0x00ff,

    GPIOE_GP0  = GPIO_GROUPE | BIT(0),GPIO_PWM0E0=GPIOE_GP0,    GPIO_SDMP=GPIOE_GP0,
    GPIOE_GP1  = GPIO_GROUPE | BIT(1),GPIO_PWM1E1=GPIOE_GP1,    GPIO_SDMN=GPIOE_GP1,
    GPIOE_GP2  = GPIO_GROUPE | BIT(2),                          GPIO_DM=GPIOE_GP2,
    GPIOE_GP3  = GPIO_GROUPE | BIT(3),                          GPIO_DP=GPIOE_GP3,
    GPIOE_GP4  = GPIO_GROUPE | BIT(4),                          GPIO_MSDO=GPIOE_GP4,
    GPIOE_GP5  = GPIO_GROUPE | BIT(5),                          GPIO_MCLK=GPIOE_GP5,
    GPIOE_GP6  = GPIO_GROUPE | BIT(6),                          GPIO_MSCN=GPIOE_GP6,
    GPIOE_GP7  = GPIO_GROUPE | BIT(7),                          GPIO_MSDI=GPIOE_GP7,
    GPIOE_ALL  = GPIO_GROUPE | 0x00ff,
} GPIO_PinTypeDef;

/**
 * @brief This function set the GPIO function of a pin.
 * @param[in]   pin the pin needs to set the GPIO function
 * @param[in]   value enable or disable the pin's GPIO function(0: disable, 1: enable)
 * @return none
 */
extern void GPIO_SetGPIOEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value);

/**
 * @brief This function set the output function of a pin.
 * @param[in]   pin the pin needs to set the output function
 * @param[in]   value enable or disable the pin's output function(0: disable, 1: enable)
 * @return none
 */
extern void GPIO_SetOutputEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value);

/**
 * @brief This function set the input function of a pin.
 * @param[in]   pin the pin needs to set the input function
 * @param[in]   value enable or disable the pin's input function(0: disable, 1: enable)
 * @return none
 */
extern void GPIO_SetInputEnable(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value);

/**
 * @brief This function determines whether the output function of a pin is enabled.
 * @param[in]   pin the pin needs to determine whether its output function is enabled.
 * @return 1: the pin's output function is enabled 0: the pin's output function is disabled
 */
extern int GPIO_IsOutputEnable(GPIO_PinTypeDef pin);

/**
 * @brief This function determines whether the input function of a pin is enabled.
 * @param[in]   pin the pin needs to determine whether its input function is enabled.
 * @return 1: the pin's input function is enabled 0: the pin's input function is disabled
 */
extern int GPIO_IsInputEnable(GPIO_PinTypeDef pin);

/**
 * @brief This function set the pin's driving strength.
 * @param[in]   pin the pin needs to set the driving strength
 * @param[in]   value the level of driving strength(1: strong 0: poor)
 * @return none
 */
extern void GPIO_SetDataStrength(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value);

/**
 * @brief This function set the pin's output level as high.
 * @param[in]   pin the pin needs to set its output level
 * @return none
 */
extern void GPIO_SetBit(GPIO_PinTypeDef pin);

/**
 * @brief This function set the pin's output level as low.
 * @param[in]   pin the pin needs to set its output level
 * @return none
 */
extern void GPIO_ResetBit(GPIO_PinTypeDef pin);

/**
 * @brief This function set the pin's output level.
 * @param[in]   pin the pin needs to set its output level
 * @param[in]   value value of the output level(1: high 0: low)
 * @return none
 */
extern void GPIO_WriteBit(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef value);

/**
 * @brief This function read the pin's input level.
 * @param[in]   pin the pin needs to read its input level
 * @return the pin's input level(1: high 0: low)
 */
extern unsigned long GPIO_ReadInputBit(GPIO_PinTypeDef pin);

/**
 * @brief This function read the pin's output level.
 * @param[in]   pin the pin needs to read its output level
 * @return the pin's output level(1: high 0: low)
 */
extern unsigned long GPIO_ReadOutputBit(GPIO_PinTypeDef pin);

/**
 * @brief This function read all the pins' input level.
 * @param[out]   p the buffer used to store all the pins' input level
 * @return none
 */
extern void GPIO_ReadAll(unsigned char *p);

/**
 * @brief This function read a pin's input level from the buffer.
 * @param[in]   pin the pin needs to read its input level from the buffer
 * @param[in]   buf the buf from which to read the pin's input level
 * @return none
 */
extern unsigned long GPIO_ReadCache(GPIO_PinTypeDef pin, unsigned char *p);

/**
 * @brief This function set a pin's pull-up/down resistor.
 * @param[in]   pin the pin needs to set its pull-up/down resistor
 * @param[in]   type the type of the pull-up/down resistor
 * @return none
 */
extern void GPIO_PullSet(GPIO_PinTypeDef pin, GPIO_PullTypeDef type);

/**
 * @brief This function enables a pin's IRQ function.
 * @param[in]   pin the pin needs to enables its IRQ function.
 * @return none
 */
extern void GPIO_EnableInterrupt(GPIO_PinTypeDef pin);

/**
 * @brief This function set a pin's polarity that trig its IRQ.
 * @param[in]   pin the pin needs to set its edge polarity that trig its IRQ
 * @param[in]   falling value of the edge polarity(1: falling edge 0: rising edge)
 * @return none
 */
extern void GPIO_SetInterruptPolarity(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling);

/**
 * @brief This function set a pin's IRQ.
 * @param[in]   pin the pin needs to enable its IRQ
 * @param[in]   falling value of the edge polarity(1: falling edge 0: rising edge)
 * @return none
 */
extern void GPIO_SetInterrupt(GPIO_PinTypeDef pin, GPIO_BitActionTypeDef falling);

/**
 * @brief This function disables a pin's IRQ function.
 * @param[in]   pin the pin needs to disables its IRQ function.
 * @return none
 */
extern void GPIO_ClearInterrupt(GPIO_PinTypeDef pin);

#if 0
/**
 * @brief This function determines whether a pin's IRQ occurs.
 * @param[in]   pin the pin needs to determine whether its IRQ occurs.
 * @return none
 */
extern int GPIO_IsInterruptOccur(GPIO_PinTypeDef pin);
#endif /* #if 0 */

#endif //GPIO_H 
