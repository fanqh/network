/********************************************************************************************************
 * @file     spi.c
 *
 * @brief    This file provides set of functions to manage the spi interface
 *
 * @author   qiuwei.chen@telink-semi.com
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
#include "bsp.h"
#include "spi.h"

//SPI related registers definitions
#define REG_SPI_DATA    REG_ADDR8(0x08)
#define REG_SPI_CTRL    REG_ADDR8(0x09)
enum {
    FLD_SPI_CTRL_CSN                = BIT(0),
    FLD_SPI_CTRL_MASTER_EN          = BIT(1),
    FLD_SPI_CTRL_OUTPUT_DIS         = BIT(2),
    FLD_SPI_CTRL_OPERATION_CMD      = BIT(3),
    FLD_SPI_CTRL_ADDR_AUTO_INCREASE = BIT(4),
    FLD_SPI_CTRL_SHARE_MODE         = BIT(5),
    FLD_SPI_CTRL_STATUS_BUSY        = BIT(6),
};

#define REG_SPI_SP      REG_ADDR8(0x0a)
enum {
    FLD_SPI_SP_CLOCK_SPEED          = BIT_RNG(0,6),
    FLD_SPI_SP_FUNCTION_EN          = BIT(7),
};

#define REG_SPI_MODE    REG_ADDR8(0x0b)
enum {
    FLD_SPI_MODE_WORK_MODE = BIT_RNG(0,1),
};

#define REG_SPI_FUNC_EN_PORTA    REG_ADDR8(0x5b0)
#define REG_SPI_FUNC_EN_PORTB    REG_ADDR8(0x5b1)

/**
 * @brief This function selects a pin port for the SPI interface
 * @param[in]   PinGrp the selected pin port
 * @return none
 */
void SPI_PinSelect(SPI_PinTypeDef PinGrp)
{
    if (PinGrp == SPI_PIN_GPIOB) {
        GPIO_SetGPIOEnable(GPIOB_GP5 | GPIOB_GP6 | GPIOB_GP7, Bit_RESET); //disable GPIO:B<5~7>
        REG_SPI_FUNC_EN_PORTB |= (BIT(5) | BIT(6) | BIT(7)); //enable SPI function:B<5~7>
        REG_SPI_FUNC_EN_PORTA &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(5)); //disable SPI function:A<2~5>
        GPIO_SetInputEnable(GPIOB_GP5, Bit_SET);
        GPIO_SetInputEnable(GPIOB_GP6, Bit_SET);
        GPIO_SetInputEnable(GPIOB_GP7, Bit_SET);
    }
    else if (PinGrp == SPI_PIN_GPIOA) {
        GPIO_SetGPIOEnable(GPIOA_GP2 | GPIOA_GP3 | GPIOA_GP4, Bit_RESET); //disable GPIO:A<2~4>
        REG_SPI_FUNC_EN_PORTA |= (BIT(2) | BIT(3) | BIT(4)); //enable SPI function:A<2~4>
        REG_SPI_FUNC_EN_PORTB &= ~(BIT(4) | BIT(5) | BIT(6) | BIT(7)); //disable SPI function:B<4~7>
        GPIO_SetInputEnable(GPIOA_GP2, Bit_SET);
        GPIO_SetInputEnable(GPIOA_GP3, Bit_SET);
        GPIO_SetInputEnable(GPIOA_GP4, Bit_SET);
    }
    REG_SPI_SP |= FLD_SPI_SP_FUNCTION_EN; //enable spi
}

/**
 * @brief This function selects a GPIO pin as one of the CS pin for SPI interface
 * @param[in]   CSPin the selected CS pin
 * @return none
 */
void SPI_CSPinSelect(GPIO_PinTypeDef CSPin)
{
    GPIO_SetGPIOEnable(CSPin, Bit_SET); //enable GPIO function
    GPIO_SetInputEnable(CSPin, Bit_RESET); //disable input function
    GPIO_SetBit(CSPin); //output high level in idle state
    GPIO_SetOutputEnable(CSPin, Bit_SET); //enable out put
}

/**
 * @brief This function configures the clock and working mode for SPI interface
 * @param[in]   DivClock the division factor for SPI module
 *              SPI clock = System clock / ((DivClock+1)*2)
 * @param[in]   Mode the selected working mode of SPI module
 * @return none
 */
void SPI_Init(unsigned char DivClock, SPI_ModeTypeDef Mode)
{
    REG_SPI_SP &= (~FLD_SPI_SP_CLOCK_SPEED);
    REG_SPI_SP |= DivClock; //spi clock=system clock/((DivClock+1)*2)

    REG_SPI_CTRL |= FLD_SPI_CTRL_MASTER_EN; //enable master mode
    REG_SPI_MODE &= (~FLD_SPI_MODE_WORK_MODE);
    REG_SPI_MODE |= Mode; //select SPI mode,surpport four modes
}

/**
 * @brief This function serves to write a bulk of data to the SPI slave
 *        device specified by the CS pin
 * @param[in]   Cmd pointer to the command bytes needed written into the 
 *              slave device first before the writing operation of actual data
 * @param[in]   CmdLen length in byte of the command bytes 
 * @param[in]   Data pointer to the data need to write 
 * @param[in]   DataLen length in byte of the data need to write
 * @param[in]   CSPin the CS pin specifing the slave device
 * @return none
 */
void SPI_Write(unsigned char *Cmd, int CmdLen, unsigned char *Data, int DataLen, GPIO_PinTypeDef CSPin)
{
    int i = 0;
    GPIO_ResetBit(CSPin); //CS level is low
    REG_SPI_CTRL &= ~FLD_SPI_CTRL_OUTPUT_DIS; //enable output
    REG_SPI_CTRL &= ~FLD_SPI_CTRL_OPERATION_CMD; //enable write
    
    /***write Cmd***/
    for (i = 0; i < CmdLen; i++) {
        REG_SPI_DATA = Cmd[i];
        while(REG_SPI_CTRL & FLD_SPI_CTRL_STATUS_BUSY); //wait writing finished
    }
    
    /***write Data***/
    for (i = 0; i < DataLen; i++) {
        REG_SPI_DATA = Data[i];
        while(REG_SPI_CTRL & FLD_SPI_CTRL_STATUS_BUSY); //wait writing finished
    }
    /***pull up CS***/
    GPIO_SetBit(CSPin); //CS level is high
}

/**
 * @brief This function serves to read a bulk of data to the SPI slave
 *        device specified by the CS pin
 * @param[in]   Cmd pointer to the command bytes needed written into the 
 *              slave device first before the reading operation of actual data
 * @param[in]   CmdLen length in byte of the command bytes 
 * @param[out]   Data pointer to the buffer that will cache the reading out data 
 * @param[in]   DataLen length in byte of the data need to read
 * @param[in]   CSPin the CS pin specifing the slave device
 * @return none
 */
void SPI_Read(unsigned char *Cmd, int CmdLen, unsigned char *Data, int DataLen, GPIO_PinTypeDef CSPin)
{
    int i = 0;
    unsigned char temp = 0;
    
    GPIO_ResetBit(CSPin); //CS level is low
    REG_SPI_CTRL &= ~FLD_SPI_CTRL_OUTPUT_DIS; //enable output

    /***write cmd***/
    for (i = 0; i < CmdLen; i++) {
        REG_SPI_DATA = Cmd[i];
        while(REG_SPI_CTRL & FLD_SPI_CTRL_STATUS_BUSY); //wait writing finished
    }

    /***when the read_bit was set 1,you can read 0x800008 to take eight clock cycle***/
    REG_SPI_CTRL |= FLD_SPI_CTRL_OPERATION_CMD; //enable read
    temp = REG_SPI_DATA; //first byte isn't useful data,only take 8 clock cycle
    while(REG_SPI_CTRL & FLD_SPI_CTRL_STATUS_BUSY); //wait reading finished
    
    /***read data***/
    for (i = 0; i < DataLen; i++) {
        Data[i] = REG_SPI_DATA; //take 8 clock cycles
        while(REG_SPI_CTRL & FLD_SPI_CTRL_STATUS_BUSY); //wait reading finished
    }
    //pull up CS
    GPIO_SetBit(CSPin); //CS level is high
}

/**
 * @brief This function configures the clock and working mode for SPI interface
 * @param[in]   DivClock the division factor for SPI module
 *              SPI clock = System clock / ((DivClock+1)*2)
 * @param[in]   Mode the selected working mode of SPI module
 * @return none
 */
void SPI_SlaveInit(unsigned char DivClock, SPI_ModeTypeDef Mode)
{
    REG_SPI_SP &= (~FLD_SPI_SP_CLOCK_SPEED);
    REG_SPI_SP |= DivClock; //spi clock=system clock/((DivClock+1)*2)

    REG_SPI_CTRL &= (~FLD_SPI_CTRL_MASTER_EN); //disable master mode
    REG_SPI_MODE &= (~FLD_SPI_MODE_WORK_MODE);
    REG_SPI_MODE |= Mode; //select SPI mode,surpport four modes
}

/**
 * @brief This function selects a pin port for the SPI interface
 * @param[in]   PinGrp the selected pin port
 * @return none
 */
void SPI_SlavePinSelect(SPI_PinTypeDef PinGrp)
{
    if (PinGrp == SPI_PIN_GPIOB) {
        GPIO_SetGPIOEnable(GPIOB_GP4 | GPIOB_GP5 | GPIOB_GP6 | GPIOB_GP7, Bit_RESET); //disable GPIO:B<4~7>
        REG_SPI_FUNC_EN_PORTB |= (BIT(4) | BIT(5) | BIT(6) | BIT(7)); //enable SPI function:B<4~7>
        REG_SPI_FUNC_EN_PORTA &= ~(BIT(2) | BIT(3) | BIT(4) | BIT(5)); //disable SPI function:A<2~5>
        GPIO_SetInputEnable(GPIOB_GP4, Bit_SET);
        GPIO_SetInputEnable(GPIOB_GP5, Bit_SET);
        GPIO_SetInputEnable(GPIOB_GP6, Bit_SET);
        GPIO_SetInputEnable(GPIOB_GP7, Bit_SET);
    }
    if (PinGrp == SPI_PIN_GPIOA) {
        GPIO_SetGPIOEnable(GPIOA_GP2 | GPIOA_GP3 | GPIOA_GP4 | GPIOA_GP5, Bit_RESET); //disable GPIO:A<2~5>
        REG_SPI_FUNC_EN_PORTA |= (BIT(2) | BIT(3) | BIT(4) | BIT(5)); //enable SPI function:A<2~5>
        REG_SPI_FUNC_EN_PORTB &= ~(BIT(4) | BIT(5) | BIT(6) | BIT(7));//disable SPI function:B<4~7>
        GPIO_SetInputEnable(GPIOA_GP2, Bit_SET);
        GPIO_SetInputEnable(GPIOA_GP3, Bit_SET);
        GPIO_SetInputEnable(GPIOA_GP4, Bit_SET);
        GPIO_SetInputEnable(GPIOA_GP5, Bit_SET);
    }  

    REG_SPI_SP |= FLD_SPI_SP_FUNCTION_EN; //enable spi
}




