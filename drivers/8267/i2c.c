/********************************************************************************************************
 * @file     i2c.c
 *
 * @brief    This file provides set of functions to manage the i2c interface
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
#include "i2c.h"
#include "gpio.h"

#define REG_I2C_FUNC_EN_PORTA    REG_ADDR8(0x5b0)
#define REG_I2C_FUNC_EN_PORTB    REG_ADDR8(0x5b1)
#define REG_I2C_FUNC_EN_PORTC    REG_ADDR8(0x5b2)

#define REG_I2C_SP               REG_ADDR8(0x00)
#define REG_I2C_ID               REG_ADDR8(0x01)
#define REG_I2C_MASTER           REG_ADDR8(0x02)
enum {
    FLD_I2C_MASTER_BUSY = BIT(0),
    FLD_I2C_MASTER_PACKET_BUSY = BIT(1),
    FLD_I2C_MASTER_RECV_STATUS = BIT(2),
};

#define REG_I2C_SCT              REG_ADDR8(0x03)
enum {
    FLD_I2C_SCT_ADDR_AUTO_INC = BIT(0),
    FLD_I2C_SCT_MASTER_EN = BIT(1),
    FLD_I2C_SCT_MAPPING_EN = BIT(2),
};

#define REG_I2C_AD               REG_ADDR8(0x04)
#define REG_I2C_DW               REG_ADDR8(0x05)
#define REG_I2C_DR               REG_ADDR8(0x06)
#define REG_I2C_CLT              REG_ADDR8(0x07)
enum {
    FLD_I2C_CLT_LAUNCH_ID = BIT(0),
    FLD_I2C_CLT_LAUNCH_ADDR = BIT(1),
    FLD_I2C_CLT_LAUNCH_CMD_WRITE = BIT(2),
    FLD_I2C_CLT_LAUNCH_CMD_READ = BIT(3),
    FLD_I2C_CLT_LAUNCH_START = BIT(4),
    FLD_I2C_CLT_LAUNCH_STOP = BIT(5),
    FLD_I2C_CLT_READ_ID_EN = BIT(6),
    FLD_I2C_CLT_ACK_EN = BIT(7),
};

#define REG_I2C_SC               REG_ADDR8(0x60)
enum {
    FLD_I2C_SC_I2C = BIT(1),
};


/**
 * @brief This function selects a pin port for I2C interface.
 * @param[in]   PinGrp the pin port selected as I2C interface pin port.
 * @return none
 */
void I2C_PinSelect(I2C_PinTypeDef PinGrp)
{
    GPIO_PinTypeDef sda;
    GPIO_PinTypeDef scl;

    switch (PinGrp) {
    case I2C_PIN_GPIOA:
        //config GPIO_PORTA as I2C
        sda = GPIOA_GP3;
        scl = GPIOA_GP4;
        GPIO_SetGPIOEnable(sda, Bit_RESET);
        GPIO_SetGPIOEnable(scl, Bit_RESET);
        REG_I2C_FUNC_EN_PORTA |= (BIT(3) | BIT(4)); //enable the i2c function of A3 and A4

        //if GPIO_PORTB's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTB & (BIT(6) | BIT(7))) { 
            REG_I2C_FUNC_EN_PORTB &= ~(BIT(6) | BIT(7)); //disable the i2c function of B6 and B7
            GPIO_SetGPIOEnable(GPIOB_GP6, Bit_SET);
            GPIO_SetGPIOEnable(GPIOB_GP7, Bit_SET);
        }

        //if GPIO_PORTC's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTC & (BIT(0) | BIT(1))) {
            REG_I2C_FUNC_EN_PORTC &= ~(BIT(0) | BIT(1)); //disable the i2c function of C0 and C1
            GPIO_SetGPIOEnable(GPIOC_GP0, Bit_SET);
            GPIO_SetGPIOEnable(GPIOC_GP1, Bit_SET);
        }
        break;
    case I2C_PIN_GPIOB:
        //config GPIO_PORTB as I2C
        sda = GPIOB_GP6;
        scl = GPIOB_GP7;
        GPIO_SetGPIOEnable(sda, Bit_RESET);
        GPIO_SetGPIOEnable(scl, Bit_RESET);
        REG_I2C_FUNC_EN_PORTB |= (BIT(6) | BIT(7)); //enable the i2c function of B6 and B7
        
        //if GPIO_PORTA's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTA & (BIT(3) | BIT(4))) {
            REG_I2C_FUNC_EN_PORTA &= ~(BIT(3) | BIT(4)); //disable the i2c function of A3 and A4
            GPIO_SetGPIOEnable(GPIOA_GP3, Bit_SET);
            GPIO_SetGPIOEnable(GPIOA_GP4, Bit_SET);
        }

        //if GPIO_PORTC's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTC & (BIT(0) | BIT(1))) {
            REG_I2C_FUNC_EN_PORTC &= ~(BIT(0) | BIT(1)); //disable the i2c function of C0 and C1
            GPIO_SetGPIOEnable(GPIOC_GP0, Bit_SET);
            GPIO_SetGPIOEnable(GPIOC_GP1, Bit_SET);
        }
        break;
    case I2C_PIN_GPIOC:
        //config GPIO_PORTC as I2C
        sda = GPIOC_GP0;
        scl = GPIOC_GP1;
        GPIO_SetGPIOEnable(sda, Bit_RESET);
        GPIO_SetGPIOEnable(scl, Bit_RESET);
        REG_I2C_FUNC_EN_PORTC |= (BIT(0) | BIT(1)); //enable the i2c function of C0 and C1
        
        //if GPIO_PORTA's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTA & (BIT(3) | BIT(4))) {
            REG_I2C_FUNC_EN_PORTA &= ~(BIT(3) | BIT(4)); //disable the i2c function of A3 and A4
            GPIO_SetGPIOEnable(GPIOA_GP3, Bit_SET);
            GPIO_SetGPIOEnable(GPIOA_GP4, Bit_SET);
        }

        //if GPIO_PORTB's I2C function is open, close it
        if (REG_I2C_FUNC_EN_PORTB & (BIT(6) | BIT(7))) { 
            REG_I2C_FUNC_EN_PORTB &= ~(BIT(6) | BIT(7)); //disable the i2c function of B6 and B7
            GPIO_SetGPIOEnable(GPIOB_GP6, Bit_SET);
            GPIO_SetGPIOEnable(GPIOB_GP7, Bit_SET);
        }
        break;
    default:
        break;
    }

    //enable input of SDA pin
    GPIO_SetInputEnable(sda, Bit_SET);

    //enable internal 10k pull-up resistors of SDA and SCL pin
    GPIO_PullSet(sda, PULL_UP_10K);
    GPIO_PullSet(scl, PULL_UP_10K);
}

/**
 * @brief This function set the id of slave device and the speed of I2C interface
 * @param[in]   SlaveID the id of slave device
 * @param[in]   DivClock the division factor of I2C clock, 
 *              I2C clock = System clock / (2*DivClock)
 * @return none
 */
void I2C_Init(unsigned char SlaveID, unsigned char DivClock)
{
    REG_I2C_SP = DivClock; //i2c clock = system_clock/2*DivClock
    REG_I2C_ID = SlaveID; //slave address
    REG_I2C_SCT |= FLD_I2C_SCT_MASTER_EN; //enable master mode
}

/**
 * @brief This function writes one byte to the slave device at the specified address
 * @param[in]   Addr pointer to the address where the one byte data will be written
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[in]   Data the one byte data will be written via I2C interface
 * @return none
 */
void I2C_WriteByte(unsigned char *Addr, int AddrLen, unsigned char Data)
{
    REG_I2C_ID &= (~BIT(0)); //SlaveID & 0xfe,.i.e write data. R:High  W:Low
    
    //set address
    if (AddrLen == 1) {
        REG_I2C_DW = Addr[0]; //address
        //lanuch start /REG_I2C_ID/REG_I2C_DW    start
        REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_ID | FLD_I2C_CLT_LAUNCH_CMD_WRITE | FLD_I2C_CLT_LAUNCH_START);
    }
    else if (AddrLen == 2) {
        REG_I2C_AD = Addr[1]; //address high
        REG_I2C_DW = Addr[0]; //address low
        //lanuch start /id/04/05    start
        REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_ID | FLD_I2C_CLT_LAUNCH_ADDR | FLD_I2C_CLT_LAUNCH_CMD_WRITE | FLD_I2C_CLT_LAUNCH_START);
    } 
    else {
        while(1);
    }
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);

    //write data
    REG_I2C_DR = Data;
    REG_I2C_CLT = FLD_I2C_CLT_LAUNCH_CMD_READ; //launch data read cycle
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);
    
    //terminate
    REG_I2C_CLT = FLD_I2C_CLT_LAUNCH_STOP; //launch stop cycle
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);
}

/**
 * @brief This function reads one byte from the slave device at the specified address
 * @param[in]   Addr pointer to the address where the one byte data will be read
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @return the one byte data read from the slave device via I2C interface
 */
unsigned char I2C_ReadByte(unsigned char *Addr, int AddrLen)
{
    unsigned char ret = 0;
    /****write address that maybe readed in,frame include:start + id + addr_H + addr_L + stop***/
    REG_I2C_ID &= (~BIT(0));
    if (AddrLen == 1) {
        REG_I2C_DW = Addr[0];
        REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_ID | FLD_I2C_CLT_LAUNCH_CMD_WRITE | FLD_I2C_CLT_LAUNCH_START); //lanuch start /id/05    start
    }
    else if (AddrLen == 2) {
        REG_I2C_AD = Addr[1]; //address high
        REG_I2C_DW = Addr[0]; //address low
        REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_ID | FLD_I2C_CLT_LAUNCH_ADDR | FLD_I2C_CLT_LAUNCH_CMD_WRITE | FLD_I2C_CLT_LAUNCH_START); //lanuch start /id/04/05    start
    } 
    else {
        while(1);
    }
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);

    /***read data from the address***/
    REG_I2C_ID |= BIT(0); //SlaveID | 0x01,.i.e read data. R:High  W:Low
    REG_I2C_SC |= FLD_I2C_SC_I2C; //reset I2C
    REG_I2C_SC &= (~FLD_I2C_SC_I2C); //clear
    REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_ID | FLD_I2C_CLT_LAUNCH_START); //start/id
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);
    //read data
    REG_I2C_CLT = (FLD_I2C_CLT_LAUNCH_CMD_READ | FLD_I2C_CLT_READ_ID_EN | FLD_I2C_CLT_ACK_EN);
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);
    ret = REG_I2C_DR;
    //termiante
    REG_I2C_CLT = FLD_I2C_CLT_LAUNCH_STOP; //launch stop cycle
    while(REG_I2C_MASTER & FLD_I2C_MASTER_BUSY);

    return ret;
}

/**
 * @brief This function writes a bulk of data to the slave device at the specified address
 * @param[in]   Addr pointer to the address where the data will be written
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[in]   pbuf pointer to the data will be written via I2C interface
 * @param[in]   len length in byte of the data will be written via I2C interface
 * @return none
 */
void I2C_Write(unsigned char *Addr, int AddrLen, unsigned char * pbuf, int len)
{
    int i = 0;
    unsigned short addr = 0;

    memcpy(&addr, Addr, AddrLen);

    for (i = 0; i < len; i++) {
        I2C_WriteByte(&addr, AddrLen, pbuf[i]);
        addr++;
    }
}

/**
 * @brief This function reads a bulk of data from the slave device at the specified address
 * @param[in]   Addr pointer to the address where the data will be read
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[out]   pbuf pointer to the buffer will cache the data read via I2C interface
 * @param[in]   len length in byte of the data will be read via I2C interface
 * @return none
 */
void I2C_Read(unsigned char *Addr, int AddrLen, unsigned char * pbuf, int len)
{
    int i = 0;
    unsigned short addr = 0;

    memcpy(&addr, Addr, AddrLen);

    for (i = 0; i < len; i++) {
        pbuf[i] = I2C_ReadByte(&addr, AddrLen);
        addr++;
    }
}
