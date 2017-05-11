/********************************************************************************************************
 * @file     i2c.h
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
#ifndef I2C_H
#define I2C_H

/**
 *  @brief  Define I2C pin port
 */
typedef enum {
    I2C_PIN_GPIOA = 0,
    I2C_PIN_GPIOB,
    I2C_PIN_GPIOC,
} I2C_PinTypeDef;

/**
 * @brief This function selects a pin port for I2C interface.
 * @param[in]   PinGrp the pin port selected as I2C interface pin port.
 * @return none
 */
extern void I2C_PinSelect(I2C_PinTypeDef PinGrp);

/**
 * @brief This function set the id of slave device and the speed of I2C interface
 * @param[in]   SlaveID the id of slave device
 * @param[in]   DivClock the division factor of I2C clock, 
 *              I2C clock = System clock / (2*DivClock)
 * @return none
 */
extern void I2C_Init(unsigned char SlaveID, unsigned char DivClock);

/**
 * @brief This function writes one byte to the slave device at the specified address
 * @param[in]   Addr pointer to the address where the one byte data will be written
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[in]   Data the one byte data will be written via I2C interface
 * @return none
 */
extern void I2C_WriteByte(unsigned char *Addr, int AddrLen, unsigned char Data);

/**
 * @brief This function reads one byte from the slave device at the specified address
 * @param[in]   Addr pointer to the address where the one byte data will be read
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @return the one byte data read from the slave device via I2C interface
 */
extern unsigned char I2C_ReadByte(unsigned char *Addr, int AddrLen);

/**
 * @brief This function writes a bulk of data to the slave device at the specified address
 * @param[in]   Addr pointer to the address where the data will be written
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[in]   Buf pointer to the data will be written via I2C interface
 * @param[in]   Len length in byte of the data will be written via I2C interface
 * @return none
 */
extern void I2C_Write(unsigned char *Addr, int AddrLen, unsigned char *Buf, int Len);

/**
 * @brief This function reads a bulk of data from the slave device at the specified address
 * @param[in]   Addr pointer to the address where the data will be read
 * @param[in]   AddrLen length in byte of the address, which makes this function is  
 *              compatible for slave device with both one-byte address and two-byte address
 * @param[out]   Buf pointer to the buffer will cache the data read via I2C interface
 * @param[in]   Len length in byte of the data will be read via I2C interface
 * @return none
 */
extern void I2C_Read(unsigned char *Addr, int AddrLen, unsigned char *Buf, int Len);

void I2C_read_bytes(unsigned char addr, unsigned char *pData, int dataSize);

#endif 
