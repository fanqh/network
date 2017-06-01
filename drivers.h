/********************************************************************************************************
 * @file     drivers.h
 *
 * @brief    This is the include header file
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
#ifndef		DRIVER_H
#define		DRIVER_H
//for 8267
#include "./drivers/8267/bsp.h"
#include "./drivers/8267/rf.h"
#include "./drivers/8267/gpio.h"
#include "./drivers/8267/irq.h"
#include "./drivers/8267/spi.h"
#include "./drivers/8267/i2c.h"
#include "./drivers/8267/pm.h"
#include "./drivers/8267/uart.h"
#include "./drivers/8267/usb_hw.h"
#include "./drivers/8267/usb_stdDescriptors.h"
#include "./drivers/8267/usb_stdRequestType.h"
#include "./drivers/8267/flash.h"
#include "./drivers/8267/adc.h"
#include "./drivers/8267/pa.h"

#endif
