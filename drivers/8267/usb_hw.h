/********************************************************************************************************
 * @file     usb_hw.h
 *
 * @brief    This file provides set of functions to manage the USB interface
 *
 * @author   xiaodong.zong@telink-semi.com; jian.zhang@telink-semi.com
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
#ifndef    USB_HW_H
#define    USB_HW_H

#include "bsp.h"

/****************************************************
 usb registers: begin  addr : 0x100
 *****************************************************/
#define REG_CTRL_EP_PTR         REG_ADDR8(0x100)
#define REG_CTRL_EP_DAT         REG_ADDR8(0x101)
#define REG_CTRL_EP_CTRL        REG_ADDR8(0x102)

// same for all endpoints


#define REG_CTRL_EP_IRQ_STA     REG_ADDR8(0x103)


#define REG_CTRL_EP_IRQ_MODE    REG_ADDR8(0x104)


#define REG_USB_CTRL            REG_ADDR8(0x105)


#define REG_USB_CYC_CALI        REG_ADDR16(0x106)
#define REG_USB_MDEV            REG_ADDR8(0x10a)
#define REG_USB_HOST_CONN       REG_ADDR8(0x10b)


#define REG_USB_SUPS_CYC_CALI   REG_ADDR8(0x10c)
#define REG_USB_INTF_ALT        REG_ADDR8(0x10d)

#define REG_USB_EP8123_PTR      REG_ADDR32(0x110)
#define REG_USB_EP8_PTR         REG_ADDR8(0x110)
#define REG_USB_EP1_PTR         REG_ADDR8(0x111)
#define REG_USB_EP2_PTR         REG_ADDR8(0x112)
#define REG_USB_EP3_PTR         REG_ADDR8(0x113)
#define REG_USB_EP4567_PTR      REG_ADDR32(0x114)
#define REG_USB_EP4_PTR         REG_ADDR8(0x114)
#define REG_USB_EP5_PTR         REG_ADDR8(0x115)
#define REG_USB_EP6_PTR         REG_ADDR8(0x116)
#define REG_USB_EP7_PTR         REG_ADDR8(0x117)
#define REG_USB_EP_PTR(i)       REG_ADDR8(0x110+((i) & 0x07))

#define REG_USB_EP8123_DAT      REG_ADDR32(0x118)
#define REG_USB_EP8_DAT         REG_ADDR8(0x118)
#define REG_USB_EP1_DAT         REG_ADDR8(0x119)
#define REG_USB_EP2_DAT         REG_ADDR8(0x11a)
#define REG_USB_EP3_DAT         REG_ADDR8(0x11b)
#define REG_USB_EP4567_DAT      REG_ADDR32(0x11c)
#define REG_USB_EP4_DAT         REG_ADDR8(0x11c)
#define REG_USB_EP5_DAT         REG_ADDR8(0x11d)
#define REG_USB_EP6_DAT         REG_ADDR8(0x11e)
#define REG_USB_EP7_DAT         REG_ADDR8(0x11f)
#define REG_USB_EP_DAT(i)       REG_ADDR8(0x118+((i) & 0x07))

#define REG_USB_EP8_CTRL        REG_ADDR8(0x120)
#define REG_USB_EP1_CTRL        REG_ADDR8(0x121)
#define REG_USB_EP2_CTRL        REG_ADDR8(0x122)
#define REG_USB_EP3_CTRL        REG_ADDR8(0x123)
#define REG_USB_EP4_CTRL        REG_ADDR8(0x124)
#define REG_USB_EP5_CTRL        REG_ADDR8(0x125)
#define REG_USB_EP6_CTRL        REG_ADDR8(0x126)
#define REG_USB_EP7_CTRL        REG_ADDR8(0x127)
#define REG_USB_EP_CTRL(i)      REG_ADDR8(0x120+((i) & 0x07))



#define REG_USB_EP8123_BUF_ADDR REG_ADDR32(0x128)
#define REG_USB_EP8_BUF_ADDR    REG_ADDR8(0x128)
#define REG_USB_EP1_BUF_ADDR    REG_ADDR8(0x129)
#define REG_USB_EP2_BUF_ADDR    REG_ADDR8(0x12a)
#define REG_USB_EP3_BUF_ADDR    REG_ADDR8(0x12b)
#define REG_USB_EP4567_BUF_ADDR REG_ADDR32(0x12c)
#define REG_USB_EP4_BUF_ADDR    REG_ADDR8(0x12c)
#define REG_USB_EP5_BUF_ADDR    REG_ADDR8(0x12d)
#define REG_USB_EP6_BUF_ADDR    REG_ADDR8(0x12e)
#define REG_USB_EP7_BUF_ADDR    REG_ADDR8(0x12f)
#define REG_USB_EP_BUF_ADDR(i)  REG_ADDR8(0x128+((i) & 0x07))

#define REG_USB_RAM_CTRL        REG_ADDR8(0x130)


#define REG_USB_ISO_MODE        REG_ADDR8(0x138)
#define REG_USB_IRQ             REG_ADDR8(0x139)
#define REG_USB_MASK            REG_ADDR8(0x13a)
#define REG_USB_EP8_SEND_MAX    REG_ADDR8(0x13b)
#define REG_USB_EP8_SEND_THRE   REG_ADDR8(0x13c)
#define REG_USB_EP8_FIFO_MODE   REG_ADDR8(0x13d)
#define REG_USB_EP_MAX_SIZE     REG_ADDR8(0x13e)

enum{
    FLD_EP_DAT_ACK   =     BIT(0),
    FLD_EP_DAT_STALL =     BIT(1),
    FLD_EP_STA_ACK   =     BIT(2),
    FLD_EP_STA_STALL =     BIT(3),
};

enum{
//    FLD_CTRL_EP_IRQ_TRANS  =    BIT_RNG(0,3),
    FLD_CTRL_EP_IRQ_SETUP  =    BIT(4),
    FLD_CTRL_EP_IRQ_DATA   =    BIT(5),
    FLD_CTRL_EP_IRQ_STA    =    BIT(6),
    FLD_CTRL_EP_IRQ_INTF   =    BIT(7),
};

enum{
    FLD_CTRL_EP_AUTO_ADDR  =    BIT(0),
    FLD_CTRL_EP_AUTO_CFG   =    BIT(1),
    FLD_CTRL_EP_AUTO_INTF  =    BIT(2),
    FLD_CTRL_EP_AUTO_STA   =    BIT(3),
    FLD_CTRL_EP_AUTO_SYN   =    BIT(4),
    FLD_CTRL_EP_AUTO_DESC  =    BIT(5),
    FLD_CTRL_EP_AUTO_FEAT  =    BIT(6),
    FLD_CTRL_EP_AUTO_STD   =    BIT(7),
};

enum{
    FLD_USB_CTRL_AUTO_CLK  =    BIT(0),
    FLD_USB_CTRL_LOW_SPD   =    BIT(1),
    FLD_USB_CTRL_LOW_JITT  =    BIT(2),
    FLD_USB_CTRL_TST_MODE  =    BIT(3),
};

enum{
    FLD_USB_MDEV_SELF_PWR  =    BIT(0),
    FLD_USB_MDEV_SUSP_STA  =    BIT(1),
};

enum{
    FLD_USB_EP_BUSY        =    BIT(0),
    FLD_USB_EP_STALL       =    BIT(1),
    FLD_USB_EP_DAT0        =    BIT(2),
    FLD_USB_EP_DAT1        =    BIT(3),
    FLD_USB_EP_MONO        =    BIT(6),
    FLD_USB_EP_EOF_ISO     =    BIT(7),
};

enum{
    FLD_USB_CEN_PWR_DN     =    BIT(0),
    FLD_USB_CLK_PWR_DN     =    BIT(1),
    FLD_USB_WEN_PWR_DN     =    BIT(3),
    FLD_USB_CEN_FUNC       =    BIT(4),
};

enum{
    FLD_USB_ENP8_FIFO_MODE =    BIT(0),
    FLD_USB_ENP8_FULL_FLAG =    BIT(1),
};

enum {
    USB_EDP_PRINTER_IN = 8, // endpoint 8 is alias of enpoint 0,  becareful.  // default hw buf len = 64
    USB_EDP_MOUSE = 2,          // default hw buf len = 8
    USB_EDP_KEYBOARD_IN = 1,    // default hw buf len = 8
    USB_EDP_KEYBOARD_OUT = 3,   // default hw buf len = 16
    USB_EDP_AUDIO_IN = 4,       // default hw buf len = 64
    USB_EDP_PRINTER_OUT = 5,    // default hw buf len = 64
    USB_EDP_SPEAKER = 6,        // default hw buf len = 16
    USB_EDP_MIC = 7,            // default hw buf len = 16
    USB_EDP_MS_IN = USB_EDP_PRINTER_IN,     // mass storage
    USB_EDP_MS_OUT = USB_EDP_PRINTER_OUT,
    USB_EDP_SOMATIC_IN = USB_EDP_AUDIO_IN,      //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
    USB_EDP_SOMATIC_OUT = USB_EDP_PRINTER_OUT,
    USB_EDP_CDC_IN = 4,
    USB_EDP_CDC_OUT = 5,
};

/**
 * @brief This function configures the fifo mode for endpoint8
 * @param[in]   dat the setting value needs to be configured 
 * @return none
 */
extern  inline  void  USBHW_Ep8FifoModeSet(unsigned char dat);

/**
 * @brief This function configures the threshold value for printer's hardware buffer
 * @param[in]   th the threshold value exceed which the content of buffer will be send out via USB
 * @return none
 */
extern  inline void USBHW_PrinterThresholdSet(unsigned char th);

/**
 * @brief This function gets the irq source of control endpoint, i.e., endpoint0
 * @param none
 * @return the irq source of control endpoint
 */
extern  inline unsigned long USBHW_CtrlEpIrqGet(void);

/**
 * @brief This function clears the irq status of control endpoint, i.e., endpoint0
 * @param[in]   irq the irq status of control endpoint
 * @return none
 */
extern  inline void USBHW_CtrlEpIrqClr(int irq);

/**
 * @brief This function sets the control register of control endpoint, i.e., endpoint0
 * @param[in]   data the value needs to set
 * @return none
 */
extern  inline void USBHW_CtrlEpCtrlWrite(unsigned char data);

/**
 * @brief This function resets the pointer of control endpoint, i.e., endpoint0
 * @param   none
 * @return none
 */
extern  inline void USBHW_CtrlEpPtrReset(void);

/**
 * @brief This function reads the data register of control endpoint, i.e., endpoint0
 * @param   none
 * @return the data read from endpoint0's data register
 */
extern  inline unsigned char USBHW_CtrlEpDataRead(void);

/**
 * @brief This function writes one byte data to the data register of control endpoint, i.e., endpoint0
 * @param[in]   data the one byte data needs to write
 * @return none
 */
extern  inline void USBHW_CtrlEpDataWrite(unsigned char data);

/**
 * @brief This function determines whether the control endpoint is busy
 * @param   none
 * @return 1: the control endpoint is busy 0: the control endpoint is idle 
 */
extern  inline unsigned char USBHW_IsCtrlEpBusy();

/**
 * @brief This function reads one byte data from the data register of the specified endpoint
 * @param[in]   ep the number of the endpoint 
 * @return one byte data read from the data register of the specified endpoint
 */
extern  inline unsigned char USBHW_EpDataRead(unsigned long ep);

/**
 * @brief This function writes one byte data to the data register of the specified endpoint
 * @param[in]   ep the number of the endpoint 
 * @param[in]   data the one byte data needs to write
 * @return none
 */
extern  inline void USBHW_EpDataWrite(unsigned long ep, unsigned char data);

/**
 * @brief This function determines whether the specified endpoint is busy
 * @param[in]   ep the number of the endpoint 
 * @return 1: the control endpoint is busy 0: the control endpoint is idle 
 */
extern  inline unsigned long USBHW_IsEpBusy(unsigned long ep);

/**
 * @brief This function enables the ack transmission when required by host
 * @param[in]   ep the number of the endpoint 
 * @return none
 */
extern  inline void USBHW_DataEpAck(unsigned long ep);

/**
 * @brief This function enables the stall function when required by host
 * @param[in]   ep the number of the endpoint 
 * @return none
 */
extern  inline void USBHW_DataEpStall(unsigned long ep);

/**
 * @brief This function resets the pointer register of specified endpoint
 * @param[in]   ep the number of the endpoint 
 * @return none
 */
extern  inline void USBHW_EpPtrReset(unsigned long ep);

/**
 * @brief This function disables the manual interrupt
 * @param[in]   m the irq mode needs to set
 * @return none
 */
extern  void USBHW_ManualInterruptDisable(int m);

/**
 * @brief This function enables the manual interrupt
 * @param[in]   m the irq mode needs to set
 * @return none
 */
extern  void USBHW_ManualInterruptEnable(int m);

/**
 * @brief This function sends a bulk of data to host via the specified endpoint
 * @param[in]   ep the number of the endpoint 
 * @param[in]   data pointer to the data need to send
 * @param[in]   len length in byte of the data need to send
 * @return none
 */
extern  void USBHW_EpWrite(unsigned long ep,unsigned char * data, int len);

/**
 * @brief This function sends two bytes data to host via the control endpoint
 * @param[in]   v the two bytes data need to send
 * @return none
 */
extern  void USBHW_CtrlEpU16Write(unsigned short v);

/**
 * @brief This function reads two bytes data from host via the control endpoint
 * @param   none
 * @return the two bytes data read from the control endpoint
 */
extern  unsigned short USBHW_CtrlEpU16Read(void);

#endif
