/********************************************************************************************************
 * @file     uart.h
 *
 * @brief    This file provides set of functions to manage the UART interface
 *
 * @author   qiuwei.chen@telink-semi.com; junjun.chen@telink-semi.com;
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
#ifndef     uart_H
#define     uart_H

/**
 *  @brief  Define parity type 
 */
typedef enum {
    PARITY_NONE = 0,
    PARITY_EVEN,
    PARITY_ODD,
} UART_ParityTypeDef;

/**
 *  @brief  Define the length of stop bit 
 */
typedef enum {
    STOP_BIT_ONE = 0,
    STOP_BIT_ONE_DOT_FIVE,
    STOP_BIT_TWO,
} UART_StopBitTypeDef;

/**
 *  @brief  Define UART RTS mode 
 */
typedef enum {
    UART_RTS_MODE_AUTO = 0,
    UART_RTS_MODE_MANUAL,
} UART_RTSModeTypeDef;

/** define the macro that configures pin port for UART interface */ 
#define    UART_GPIO_CFG_PA6_PA7()  do{\
                                        *(volatile unsigned char  *)0x800586 &= 0x3f;\
                                        *(volatile unsigned char  *)0x8005b0 |= 0x80;\
                                    }while(0)
/** define the macro that configures pin port for UART interface */ 
#define    UART_GPIO_CFG_PB2_PB3()  do{\
                                        *(volatile unsigned char  *)0x80058e &= 0xf3;\
                                        *(volatile unsigned char  *)0x8005b1 |= 0x0c;\
                                    }while(0)

/** define the macro that configures pin port for UART interface */ 
#define    UART_GPIO_CFG_PC2_PC3()  do{\
                                        *(volatile unsigned char  *)0x800596 &= 0xf3;\
                                        *(volatile unsigned char  *)0x8005b2 |= 0x0c;\
                                    }while(0)
                                    
/**
 * @brief This function initializes the UART module.
 * @param[in]   BaudRate the selected baudrate for UART interface
 * @param[in]   Parity selected parity type for UART interface
 * @param[in]   StopBit selected length of stop bit for UART interface
 * @return none
 */
extern void UART_Init(unsigned int BaudRate, UART_ParityTypeDef Parity, UART_StopBitTypeDef StopBit);

/**
 * @brief uart send data function, this  function tell the DMA to get data from the RAM and start
 *        the DMA transmission
 * @param[in]   addr pointer to the buffer containing data need to send
 * @return '1' send success; '0' DMA busy
 */
extern unsigned char UART_Send(unsigned char* Addr);

/**
 * @brief data receive buffer initiate function. DMA would move received uart data to the address space,
 *        uart packet length needs to be no larger than (recBuffLen - 4).
 * @param[in]   RecvAddr pointer to the receiving buffer
 * @param[in]   RecvBufLen length in byte of the receiving buffer
 * @return none
 */
extern void UART_RecBuffInit(unsigned char *RecvAddr, unsigned short RecvBufLen);

/**
 * @brief This function determines whether parity error occurs once a packet arrives.
 * @return 1: parity error 0: no parity error
 */
extern unsigned char UART_IsParityError(void);

/**
 * @brief This function clears parity error status once when it occurs.
 * @return none
 */
extern void UART_ClearParityError(void);

/**
 * @brief UART hardware flow control configuration. Configure RTS pin.
 * @param[in]   Enable: enable or disable RTS function.
 * @param[in]   Mode: set the mode of RTS(auto or manual).
 * @param[in]   Thresh: threshold of trig RTS pin's level toggle(only for auto mode), 
 *                     it means the number of bytes that has arrived in Rx buf.
 * @param[in]   Invert: whether invert the output of RTS pin(only for auto mode)
 * @return none
 */
extern void UART_RTSCfg(unsigned char Enable, UART_RTSModeTypeDef Mode, unsigned char Thresh, unsigned char Invert);

/**
 * @brief This function sets the RTS pin's level manually
 * @param[in]   Polarity: set the output of RTS pin(only for manual mode)
 * @return none
 */
extern void UART_RTSLvlSet(unsigned char Polarity);

/**
 * @brief UART hardware flow control configuration. Configure CTS pin.
 * @param[in]   Enable: enable or disable CTS function.
 * @param[in]   Select: when CTS's input equals to select, tx will be stopped
 * @return none
 */
extern void UART_CTSCfg(unsigned char Enable, unsigned char Select);

volatile unsigned char UART_Send_Byte(unsigned char c);

#endif
