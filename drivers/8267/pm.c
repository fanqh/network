/********************************************************************************************************
 * @file     pm.c
 *
 * @brief    This file provides set of functions for power management module
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
#include "pm.h"

//PM related registers definition
#define REG_PM_SYSTIMER_TICK    REG_ADDR32(0x740)
#define REG_PM_SYSTIMER_CTRL    REG_ADDR8(0x74c)
enum {
    FLD_PM_SYSTIMER_IRQ_MASK = BIT(1),
    FLD_PM_SYSTIMER_32K_MANUAL = BIT(2),
    FLD_PM_SYSTIMER_32K_MODE = BIT(3),
    FLD_PM_SYSTIMER_SS_EN = BIT(4),
    FLD_PM_SYSTIMER_SUSPEND_BYPASS = BIT(5),
    FLD_PM_SYSTIMER_32K_SET = BIT(6),
    FLD_PM_SYSTIMER_32K_CAL_EN = BIT(7),
};

#define REG_PM_SYSTIMER_TRIG    REG_ADDR8(0x74f)
enum {
    FLD_PM_SYSTIMER_PREPARE_SUSPEND = BIT(1),
    FLD_PM_SYSTIMER_32K_MANUAL_SYNC = BIT(3),
};

#define REG_PM_SYSTIMER_32K_CAL_CNT    REG_ADDR16(0x750)
#define REG_PM_SYSTIMER_32K_TICK    REG_ADDR32(0x754)

#define REG_PM_MSPI_DATA    REG_ADDR8(0xc)
#define REG_PM_MSPI_CTRL    REG_ADDR8(0xd)
enum {
    FLD_PM_MSPI_MCSN = BIT(0),
    FLD_PM_MSPI_MSDO = BIT(1),
    FLD_PM_MSPI_CONTINUOUS_MODE = BIT(2),
    FLD_PM_MSPI_CMD = BIT(3),
    FLD_PM_MSPI_ADDR_AUTO = BIT(4),
};

#define REG_PM_GPIOB_OUTPUT    REG_ADDR8(0x58b)
#define REG_PM_GPIOE_IE    REG_ADDR8(0x5a1)
#define REG_PM_GPIO_CTRL    REG_ADDR8(0x5b5)
enum {
    FLD_PM_GPIO_WAKEUP_EN = BIT(2),
    FLD_PM_GPIO_IRQ_EN = BIT(3),
};

#define REG_PM_IRQ_EN    REG_ADDR8(0x643)

#define REG_PM_XTAL_INPUT_SET    REG_ADDR8(0x28)
#define REG_PM_CLOCK_SEL    REG_ADDR8(0x66)
#define REG_PM_WAKEUP_DIG_SRC    REG_ADDR8(0x6e)
#define REG_PM_POWER_DOWN_CTRL    REG_ADDR8(0x6f)
#define REG_PM_32K_SRC_SET    REG_ADDR8(0x73)
#define REG_PM_WAKEUP_QDEC_EN    REG_ADDR8(0xd4)
#define REG_PM_WAKEUP_QDEC_IRQ    REG_ADDR8(0xd5)

enum {
    FLD_PM_SUSPEND_EN = BIT(0),
    FLD_PM_RESET_ALL = BIT(5),
    FLD_PM_LOW_POWER_MODE = BIT(6),
    FLD_PM_STALL_MCU = BIT(7),
};

#define ANA_REG_PM_32K_XTAL_CSEL    0x01
#define ANA_REG_PM_PWDOWN_CTRL    0x05
#define ANA_REG_PM_DCDC_DELAY_TIME    0x20
#define ANA_REG_PM_PA_WAKEUP_POL    0x21
#define ANA_REG_PM_PA_WAKEUP_EN    0x27
#define ANA_REG_PM_WAKEUP_SOURCE    0x26
#define ANA_REG_PM_WAKEUP_STATUS    0x44
#define ANA_REG_PM_PWDOWN_MASK    0x2c
enum {
    FLD_PM_PWDOWN_32K_RC = BIT(0),
    FLD_PM_PWDOWN_32K_PAD = BIT(1),
    FLD_PM_PWDOWN_16M_PAD = BIT(2),
    FLD_PM_PWDOWN_LDO = BIT(4),
    FLD_PM_PWDOWN_LOW_LEAKAGE = BIT(5),
    FLD_PM_PWDOWN_EN = BIT(6),
    FLD_PM_ISO_EN = BIT(7),
};
#define ANA_REG_PM_XTAL_CTRL    0x2d

#define PM_DC_RESET_DURATION     2200 //us
#define PM_WAKE_UP_MARGIN        2300 //us
#define PM_SLEEP_DURATION_MIN    2600 //us
#define PM_SLEEP_DURATION_MAX    0xc0000000 //sys ticks

#define SWAP_BIT0_BIT6(x)     ( ((x)&0xbe) | ( ((x)&0x01)<<6 ) | ( ((x)&0x40)>>6 )  )

//restore analog Regs' value as default
static const BSP_TblCmdSetTypeDef tbl_cpu_wakeup_init[] = {
    0x0060, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst
    0x0061, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst
    0x0062, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst
    0x0063, 0xff,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst
    0x0064, 0xff,  TCMD_UNDER_BOTH | TCMD_WRITE,
    //0x0065, 0xff,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst

    // 0x0067, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst
    // 0x0066, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //open all the clk,disable all the rst

    //0x0073, 0x00,  TCMD_UNDER_BOTH | TCMD_WRITE, //low power divider disable
    0x0073, 0x04,  TCMD_UNDER_BOTH | TCMD_WRITE, //low power divider disable: 0x04 for 0x8267
    0x0620, 0x01,  TCMD_UNDER_BOTH | TCMD_WRITE,
    0x074f, 0x01,  TCMD_UNDER_BOTH | TCMD_WRITE,

    // 0x01, 0x4e,  TCMD_UNDER_BOTH | TCMD_WAREG,
    // 0x2c, 0x00,  TCMD_UNDER_BOTH | TCMD_WAREG,
    //0x81, 0xc0,  TCMD_UNDER_BOTH | TCMD_WAREG, //xtal cap off to quick settle osc
    0x20, 0xc1,  TCMD_UNDER_BOTH | TCMD_WAREG,   //wakeup reset time: 2000us(bit0 and bit6 are swapped each other):c1:2000;cf:1500
    0x2d, 0x0f,  TCMD_UNDER_BOTH | TCMD_WAREG, //quick settle: 200 us
    // 0x2d, 0x4f,  TCMD_UNDER_BOTH | TCMD_WAREG, //quick settle: 200 us, 32k clock source external 32k pad

    // 0x2f, 0xcd,  TCMD_UNDER_BOTH | TCMD_WAREG, //cal wait len
    // 0x05, 0x60,  TCMD_UNDER_BOTH | TCMD_WAREG, //turn on crystal pad,  bit7: bbpll  (8267)
    0x05, 0x62,  TCMD_UNDER_BOTH | TCMD_WAREG, //turn off crystal pad,  bit7: bbpll  (8267)
    0x88, 0x0f,  TCMD_UNDER_BOTH | TCMD_WAREG, //bit[1:0]: 192M CLOCK to digital   (8267)

    // 0x01, 0xce,  TCMD_UNDER_BOTH | TCMD_WAREG, //open capacitor for external 32K xtal
    0x01, 0x4e,  TCMD_UNDER_BOTH | TCMD_WAREG,
};

__attribute__((section(".ram_code"))) static unsigned int PM_Get32kTick()
{
    unsigned int t0 = 0;
    unsigned int t1 = 0;
    unsigned int n = 0;

    //system timer manual mode
    REG_PM_SYSTIMER_CTRL = (FLD_PM_SYSTIMER_32K_MODE | FLD_PM_SYSTIMER_SUSPEND_BYPASS);
    while (1) {
        //start read  
        REG_PM_SYSTIMER_TRIG = FLD_PM_SYSTIMER_32K_MANUAL_SYNC;                       
        while (REG_PM_SYSTIMER_TRIG & FLD_PM_SYSTIMER_32K_MANUAL_SYNC);
        t0 = t1;
        t1 = REG_PM_SYSTIMER_32K_TICK;
        if (n) {
            if ((unsigned int)(t1 - t0) < 2) {
                return t1;
            }
            else if ( (t0^t1) == 1 ) {
                return t0;
            }
        }
        n++;
    }
    return t1;
}

__attribute__((section(".ram_code"))) static void pm_start(void)
{
    volatile unsigned int i;

    REG_PM_MSPI_CTRL = 0;
    REG_PM_MSPI_DATA = 0xb9;
    for(i = 0; i < 2; i++); //1440ns when 32M clk
    REG_PM_MSPI_CTRL = 1;

    REG_PM_GPIOB_OUTPUT &= (~BIT(0)); //SWS dataO -> 0 , close digital pullup
    REG_PM_GPIOE_IE &= 0x0f; //MSPI ie disable
    REG_PM_POWER_DOWN_CTRL = FLD_PM_SUSPEND_EN | FLD_PM_STALL_MCU;
    for(i = 0; i < 0x30; i++);
    REG_PM_GPIOE_IE |= 0xf0;
    REG_PM_GPIOB_OUTPUT |= BIT(0);

    REG_PM_MSPI_CTRL = 0;
    REG_PM_MSPI_DATA = 0xab;
    for(i = 0; i < 2; i++); //1440ns when 32M clk
    REG_PM_MSPI_CTRL = 1;
}

/**
 * @brief This function serves to initialize the related analog registers
 *        to default values after MCU is waked up from deep sleep mode.
 * @return none
 */
void PM_WakeupInit(void)
{
    LoadTblCmdSet(tbl_cpu_wakeup_init, sizeof(tbl_cpu_wakeup_init) / sizeof(BSP_TblCmdSetTypeDef));
    REG_PM_SYSTIMER_32K_CAL_CNT = 0x0001;
    REG_PM_SYSTIMER_TRIG = 0x01;
    while (REG_PM_SYSTIMER_32K_CAL_CNT == 0x0001); //wait for 32KTimer's cal completed
}

/**
 * @brief This function configures a pad pin as the wakeup pin.
 * @param[in]   pin the pin needs to be configured as wakeup pin
 * @param[in]   pol the wakeup polarity of the pad pin(0: low-level wakeup, 1: high-level wakeup)
 * @param[in]   en enable or disable the wakeup function for the pan pin(1: Enable, 0: Disable) 
 * @return none
 */
void PM_PadSet(Pin_TypeDef pin, int pol, int en)
{
    unsigned char mask = pin & 0xff;
    unsigned char areg;
    unsigned char val;

    if(!mask)
        return;

    /***set pad pin's polarity***/
    areg = ((pin>>8) + ANA_REG_PM_PA_WAKEUP_POL);
    val = ReadAnalogReg(areg);
    if (pol) {
        val &= ~mask;
    }
    else {
        val |= mask;
    }
    WriteAnalogReg (areg, val);

    /***store enabled pad pin***/
    areg = ((pin>>8) + ANA_REG_PM_PA_WAKEUP_EN);
    val = ReadAnalogReg(areg);
    if (en) {
        val |= mask;
    }
    else {
        val &= ~mask;
    }
    WriteAnalogReg(areg, val);
}

/**
 * @brief This function configures a GPIO pin as the wakeup pin.
 * @param[in]   pin the pin needs to be configured as wakeup pin
 * @param[in]   pol the wakeup polarity of the pad pin(0: low-level wakeup, 1: high-level wakeup)
 * @param[in]   en enable or disable the wakeup function for the pan pin(1: Enable, 0: Disable) 
 * @return none
 */
void PM_GPIOSet(Pin_TypeDef pin, int pol, int en)    //only surport from suspend
{
    if ((pin&0xfff) == 0) {
        return;
    }

    unsigned char bit = pin & 0xff;
    unsigned long reg_en = IOBASE + 0x587 + ((pin>>8)<<3);
    unsigned long reg_pol = IOBASE + 0x584 + ((pin>>8)<<3);

    if (en) {                                         //set gpio pin wakeup enable
        WRITE_REG8(reg_en,READ_REG8(reg_en)|bit);
    }
    else {
        WRITE_REG8(reg_en,READ_REG8(reg_en)&(~bit));
    }

    if (pol) {                                        //set gpio pin wakeup polarity
        WRITE_REG8(reg_pol,READ_REG8(reg_pol)&(~bit));
    }
    else {
        WRITE_REG8(reg_pol,READ_REG8(reg_pol)|bit);
    }

    REG_PM_GPIO_CTRL |= 0x04; //0x5b5[2] gpio wakeup enable
}

/**
 * @brief This function makes the MCU actually enter low power mode.
 * @param[in]   DeepSleep set whether to enter deep sleep mode(1: enter deep sleep, 0: enter suspend)
 * @param[in]   WakeupSrc set wakeup source
 * @param[in]   WakeupTick set the tick value of System_Timer at the wakeup moment when 
 *              32k Timer is set as a wakeup source 
 * @return sources that wake the MCU up from low power mode
 */
__attribute__((section(".ram_code"))) int PM_LowPwrEnter(int DeepSleep, int WakeupSrc, unsigned long WakeupTick)
{


#if 1
    int system_tick_enable = WakeupSrc & WAKEUP_SRC_TIMER;
    unsigned int span = (unsigned int)(WakeupTick - ClockTime());
    unsigned char qdec_wakeup_en = 0;
    unsigned int tick_cur_32k;
    unsigned int tick_cur;
    unsigned short cali_32k_cnt = REG_PM_SYSTIMER_32K_CAL_CNT;

    if (system_tick_enable) {
        if (span > PM_SLEEP_DURATION_MAX) {
            return  ReadAnalogReg(ANA_REG_PM_WAKEUP_STATUS) & 0x0f;
        }
        else if (span < (PM_SLEEP_DURATION_MIN * TickPerUs)) {
            //if duration of sleep is too short, do empty loop instead of actual sleep
            unsigned long t = ClockTime();
            WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f); //clear all status
            unsigned char st;
            do {
                st = ReadAnalogReg (ANA_REG_PM_WAKEUP_STATUS) & 0x0f;
            } while (((unsigned int)(ClockTime() - t) < span) && !st);
            return st;
        }
    } 

    unsigned char r = REG_PM_IRQ_EN; //irq disable
    REG_PM_IRQ_EN = 0;

    /******set wakeup source ******/
    WriteAnalogReg(ANA_REG_PM_WAKEUP_SOURCE, (WakeupSrc & 0xff));

    unsigned char anareg2c = 0;        //power down control
    unsigned char anareg2d = ReadAnalogReg(ANA_REG_PM_XTAL_CTRL);        //power down control

    REG_PM_WAKEUP_DIG_SRC = 0x00;
    if (WakeupSrc >> 8) {                               //usb/digital_gpio/qdec wakeup
        REG_PM_WAKEUP_DIG_SRC = (WakeupSrc >> 8);
        if ((WakeupSrc>>8) & 0x10) {                    //if enable qdec wankeup, set 0xd4[0] to 1
            REG_PM_WAKEUP_QDEC_EN |= BIT(0);
            qdec_wakeup_en = 1;
        }
    }
    WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f);                //clear all flag
    
    /****** set power down ******/
    if (system_tick_enable) {
        anareg2c &= 0xfc; //32K RC need be enabled
        // anareg2c &= 0xfe;
    }
    else {
        REG_PM_SYSTIMER_CTRL = 0x20;
        anareg2c |= BIT(0);
        anareg2c |= BIT(1);
    }

    if (qdec_wakeup_en) {                       //32K RC need be enabled
        anareg2c &= 0xfc;
        // anareg2c &= 0xfe;
    }

    /****** change to 32M RC clock before suspend ******/
    unsigned char reg66 = REG_PM_CLOCK_SEL;
    REG_PM_CLOCK_SEL = 0;

    WriteAnalogReg(ANA_REG_PM_PWDOWN_MASK, (DeepSleep ? 0xfc : 0x5e) | anareg2c);
    // WriteAnalogReg(0x2c, (DeepSleep ? 0xfe : 0x5e) | anareg2c);
    WriteAnalogReg(ANA_REG_PM_XTAL_CTRL, anareg2d & 0xdf); 

    //set DC rst duration
    span = PM_DC_RESET_DURATION*TickPerUs;
    unsigned int M = span / cali_32k_cnt;
    unsigned int m = span % cali_32k_cnt;
    span = M*16 + m/(cali_32k_cnt/16);
    unsigned char rst_cycle =  0xff - span;
    WriteAnalogReg(ANA_REG_PM_DCDC_DELAY_TIME, SWAP_BIT0_BIT6(rst_cycle));

    //set 32k wakeup tick
    tick_cur_32k = PM_Get32kTick();
    tick_cur = ClockTime();
    span = (unsigned int)(WakeupTick - tick_cur) - PM_WAKE_UP_MARGIN*TickPerUs;
    M = span / cali_32k_cnt;
    m = span % cali_32k_cnt;
    span = M*16 + m/(cali_32k_cnt/16);
    REG_PM_SYSTIMER_CTRL = 0x2c;
    REG_PM_SYSTIMER_32K_TICK = (tick_cur_32k + span);
    REG_PM_SYSTIMER_TRIG = BIT(3);
    while (REG_PM_SYSTIMER_TRIG & BIT(3)); 
    WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f); 

    pm_start();

    if (DeepSleep) {
        REG_PM_POWER_DOWN_CTRL = 0x20; //reboot
    }

    /*suspend recover setting*/
    WriteAnalogReg(ANA_REG_PM_PWDOWN_MASK, 0x00);

    //recover system clock from 32k clock
    span = (unsigned int)(PM_Get32kTick() - tick_cur_32k);
    M = span / 16;
    m = span % 16;
    span = M*cali_32k_cnt + m*cali_32k_cnt/16;
    tick_cur += span;

    /***clear "qdec_irq" irq source flag***/
    if (REG_PM_WAKEUP_QDEC_IRQ & 0x01) {  
        REG_PM_WAKEUP_QDEC_IRQ |= 0x01;
        while(REG_PM_WAKEUP_QDEC_IRQ & 0x01);
    }

    /******system clock has to be on******/
    REG_PM_CLOCK_SEL = reg66; //restore system clock
    REG_PM_SYSTIMER_TICK = tick_cur; //recover system tick
    REG_PM_SYSTIMER_CTRL = 0x90; //auto mode and enable 32K rc calibration
    REG_PM_SYSTIMER_TRIG = 0x01; //enable system timer

    unsigned char anareg44 = ReadAnalogReg(ANA_REG_PM_WAKEUP_STATUS);
    if (anareg44 & BIT(1)) { //wakeup from timer
        while ((unsigned int)(ClockTime() - WakeupTick) > BIT(30));
    }

    REG_PM_IRQ_EN = r; //restore the irq
    return anareg44;
#else
	while(ClockTime()- WakeupTick >=BIT(31));
	return 0;

#endif
}

/**
 * @brief This function return the source mask that wakeup the low power mode.
 * @param   none
 * @return sources that wake the MCU up from low power mode
 */
unsigned char PM_WakeUpStatusGet(void)
{
    unsigned char ret = ReadAnalogReg(ANA_REG_PM_WAKEUP_STATUS);
    return ret;
}

/**
 * @brief This function clear the wakeup source status register
 * @param[in]   Mask the mask of the bits need to clear 
 * @return none
 */
void PM_WakeUpStatusClear(unsigned char Mask)
{
    WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, Mask);
}

/**
 * @brief This function serves to enable the 32k external crystal as source of 32K Timer. 
 * @return
 */
void PM_32kPadEnable()
{
    //turn on external 32k pad
    WriteAnalogReg(ANA_REG_PM_PWDOWN_CTRL, ReadAnalogReg(ANA_REG_PM_PWDOWN_CTRL) & (~BIT(1)));

    //turn on external 32k pad's capacitors
    WriteAnalogReg(ANA_REG_PM_32K_XTAL_CSEL, ReadAnalogReg(ANA_REG_PM_32K_XTAL_CSEL) | BIT(7));

    //set 32k clock source as external 32k pad
    WriteAnalogReg(ANA_REG_PM_XTAL_CTRL, ReadAnalogReg(ANA_REG_PM_XTAL_CTRL) | BIT(6));

    //set PC2/PC3 as external crystal's input pin
    REG_PM_XTAL_INPUT_SET |= BIT(0);

    //set 32k clock's source as external 32k pad
    REG_PM_32K_SRC_SET |= BIT(0);

    //wait for external 32k pad settle down
    WaitMs(3000);
}

//definitions for U64 operations
typedef struct {            /*The result of the multiplication with 64 bits.*/
    unsigned int High32;    /*The high 32 bits.*/
    unsigned int Low32;     /*The low 32 bits.*/
} U64_VALUE_t;

/** Get the high word and low word of a variable with 32 bits.*/
#define LOWORD(p)           ((p) & 0x0000FFFFU)
#define HIWORD(p)           (((p) >> 16) & 0x0000FFFFU)

#define MAXVALUEOF32BITS    (0xFFFFFFFFU)

/**
* @brief Calculate the multiplication between two factors,
*        and one factor is the high 32 bit of one 64 bit unsigned interger and the other factor is 32 bit unsigned interger.
* @param Factor64H           [IN]The high 32 bit of one 64 bit unsigned interger.
*                               - Type: unsigned int  \n
*                               - Range: [0,0xFFFFFFFF].\n
* @param Factor32            [IN]A 32 bit unsigned interger.
*                               - Type: unsigned int \n
*                               - Range: [0,0xFFFFFFFF].\n
* @param Result          [OUT]A pointer to the production between the two factors.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
*
* @return The overflow status of the multiplication.
*                               - Type: unsigned char \n
*                               - Range: (0, 1). \n
*
*/
__attribute__((section(".ram_code"))) static unsigned char TN_Cal64Multi32_H(
    const unsigned int Factor64H,                /*The high 32 bit of one 64 bit unsigned interger.*/
    const unsigned int Factor32,               /*A 32 bit unsigned interger.*/
    U64_VALUE_t * const Result)       /*A pointer to the production between the two factors.*/

{
    unsigned char IsOverFlow = 0;   /*The overflow state of the product.*/
    if (0 != Result) {    /*Check the output buffer.*/
        /*Initiate the output buffer.*/
        Result->High32 = 0U;
        Result->Low32 = 0U;

        if ((0U == Factor64H) || (0U == Factor32)) {/*Check whether one of the multiplier factor is zero.*/
            /*Set the product to zero when one of the multiplier factor is zero.*/
            Result->High32 = 0U;
            Result->Low32 = 0U;
        }
        else {
            if ((Factor64H * Factor32) / Factor32 == Factor64H) {/*Check whether the product is not overflow.*/
                /*Calculate the product when the product is not overflow*/
                Result->High32 = 0U;
                Result->Low32 = Factor64H * Factor32;
            }
            else {
                /*Set the overflow flag of the product and print the overflow message for debug.*/
                IsOverFlow = 1;
            }
        }
    }
    return IsOverFlow;
}

/**
* @brief Calculate the multiplication between two factors,
*        and one factor is the low 32 bit of one 64 bit unsigned interger and the other factor is 32 bit unsigned interger.
* @param Factor64L             [IN]The low 32 bit of one 64 bit unsigned interger
*                               - Type: unsigned int  \n
*                               - Range: [0,0xFFFFFFFF].\n
* @param Factor32               [IN]A 32 bit unsigned interger.
*                               - Type: unsigned int \n
*                               - Range: [0,0xFFFFFFFF].\n
* @param Result          [OUT]A pointer to the production between the two factors.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
*
* @return none
*/
__attribute__((section(".ram_code"))) static void TN_Cal64Multi32_L(
    const unsigned int Factor64L,            /*The low 32 bit of one 64 bit unsigned interger.*/
    const unsigned int Factor32,              /*An 32 bit unsigned interger.*/
    U64_VALUE_t * const Result)   /*A pointer to the production between the two factors.*/

{
    unsigned int  A0    = 0U;   /*The low 16 bits of the multiplier factor with 64 bits.*/
    unsigned int  A1    = 0U;   /*The 16~31 bits of the multiplier factor with 64 bits.*/
    unsigned int  B0    = 0U;   /*The low 16 bits of the multiplier factor with 32 bits.*/
    unsigned int  B1    = 0U;   /*The 16~31 bits of the multiplier factor with 32 bits.*/
    unsigned int  A0B0  = 0U;   /*The product between A0 and B0.*/
    unsigned int  A1B0  = 0U;   /*The product between A1 and B0.*/
    unsigned int  A0B1  = 0U;   /*The product between A0 and B1.*/
    unsigned int  A1B1  = 0U;   /*The product between A1 and B1.*/
    if (0 != Result)   /*Check the output buffer.*/
    {
        /*Initiate the output buffer.*/
        Result->High32 = 0U;
        Result->Low32 = 0U;
        if((0U == Factor64L) || (0U == Factor32)) /*Check whether one of the multiplier factor with low 32 bits is zero.*/
        {
            /*Set the product to zero when one of the multiplier factor with 32 bits is zero.*/
            Result->High32 = 0U;
            Result->Low32 = 0U;
        }
        else
        {
            if((Factor32 * Factor64L )/Factor32   ==   Factor64L )/*Check whether the product between two multiplier factor with 32 bits is not oveeflow.*/
            {
                /*Calculate the product when the product is not overflow.*/
                Result->High32 = 0U;
                Result->Low32 = Factor64L  * Factor32;
            }
            else
            {
                /*Calculate the product when the product between two multiplier factor with 32 bits is not overflow.*/
                A0 = LOWORD( Factor64L );    /*Get the low 16 bits of the multiplier factor with 64 bits.*/
                A1 = HIWORD( Factor64L );    /*Get the 16~31 bits of the multiplier factor with 64 bits.*/
                B0 = LOWORD( Factor32 );       /*Get the low 16 bits of the multiplier factor with 32 bits.*/
                B1 = HIWORD( Factor32 );       /*Get the 16~31 bits of the multiplier factor with 32 bits.*/
                /*Calculate the product high 32 bit.*/
                /* Factor64L * Factor32 = (A0+A1*2^16)*(B0+B1*2^16) = A0*B0 + (A0*B1+A1*B0)*2^16 + A1*B1*2^32 */
                A0B0 = A0 * B0;
                A1B0 = A1 * B0 + HIWORD( A0B0 );
                A0B1 = A0 * B1 + LOWORD( A1B0 );
                A1B1 = A1 * B1 + HIWORD( A1B0 ) + HIWORD( A0B1 );
                /*Calculate the product low 32 bit.*/
                Result->High32 = A1B1;
                Result->Low32 = Factor64L * Factor32;
            }
        }
    }

}

/**
* @brief   Calculate the addition between the two 64 bit factors.
* @param Factor32             [IN]A pointer to one factor of the addition.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param Factor32             [IN]A pointer to another factor of the addition.
*                               - Type: U64_VALUE_t *\n
*                               - Range: N/A.\n
* @param Result              [OUT]A pointer to the result of the addition.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
*
* @return The overflow status of the addition.
*                               - Type: unsigned char \n
*                               - Range: (0, 1). \n
*
*/
__attribute__((section(".ram_code"))) static unsigned char TN_Cal64Add(
    const U64_VALUE_t * const Factor1,    /*One factor of the addition.*/
    const U64_VALUE_t * const Factor2,    /*Another factor of the addition.*/
    U64_VALUE_t * const Result)           /*The result of the addition.*/

{
    unsigned char IsOverFlow = 0;   /*The overflow state of the product.*/
    if (0 != Result) {  /*Check the output buffer.*/
        /*Initiate the output buffer.*/
        Result->High32 = 0U;
        Result->Low32 = 0U;
        if ((0 != Factor1) && (0 != Factor2)) {  /*Check the input buffer.*/
            if( (Factor1->Low32) > ((unsigned int)(0xFFFFFFFFu) - (Factor2->High32)) ) { /*Check whether the high 32 bits of the product is overflow.*/
                /*Set the overflow flag of the product and print the overflow message for debug.*/
                IsOverFlow = 1;
            }
            else {
                /*Calculate the final product and reset the overflow state.*/
                Result->High32 = Factor2->High32 + Factor1->Low32;
                Result->Low32 = Factor2->Low32;
                IsOverFlow = 0;
            }
        }
    }
    return IsOverFlow;
}

/**
* @brief Calculate the multiplication between two factors, and one factor is 64 bit unsigned interger while the other factor 32 bit.
* @param U64Factor           [IN]One of the multiplier factor with 64 bits.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param U32Factor           [IN]One of the multiplier factor with 32 bits.
*                               - Type: unsigned int \n
*                               - Range: [0,0xFFFFFFFF]\n
* @param Result              [OUT]A pointer to the multiplication result with 64 bits.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
*
* @return The overflow status of the multiplication.
*                               - Type: unsigned char \n
*                               - Range: (0, 1). \n
*
*/
__attribute__((section(".ram_code"))) static unsigned char TN_UINT64Multiply(
    const U64_VALUE_t * const U64Factor,      /*One of the multiplier factor with 64 bits.*/
    const unsigned int U32Factor,             /*One of the multiplier factor with 32 bits.*/
    U64_VALUE_t * const Result)               /*The multiplication result with 64 bits.*/

{
    unsigned char IsOverFlow = 0;       /*The overflow state of the U64Factor.*/
    U64_VALUE_t Product64H_32 = {0};    /*The U64Factor between the high 32 bits of the multiplier factor with 64 bits and the multiplier factor with 32 bits.*/
    U64_VALUE_t Product64L_32 = {0};    /*The U64Factor between the low 32 bits of the multiplier factor with 64 bits and the multiplier factor with 32 bits.*/
    /*Initiate the output buffer.*/
    if (0 != Result) { /*Check the output buffer.*/
        Result->High32 = 0U;
        Result->Low32 = 0U;
        if (0 != U64Factor) {  /*Check the input buffer.*/
            if ((0U == U32Factor)||((0U == U64Factor->High32) && (0U == U64Factor->Low32))) { /*Check whether one of the multiplier factor is zero.*/
                /*The result is zero when one of the multiplier factor is zero.*/
                Result->High32 = 0U;
                Result->Low32 = 0U;
                IsOverFlow = 0;
            }
            else {
                /*Calculate the multiplication between the high 32 bits of the multiplier factor with 64 bits and the multiplier factor with 32 bits.*/
                IsOverFlow = TN_Cal64Multi32_H(U64Factor->High32,U32Factor,&Product64H_32);
                if (0 == IsOverFlow) {/*Check whether the multiplication between the high 32 bits of the 64 bits multiplier factor and the 32 bits multiplier factor is not overflow.*/
                    /*Calculate multiplication between the low 32 bits of the two multiplier factors.*/
                    TN_Cal64Multi32_L(U64Factor->Low32,U32Factor,&Product64L_32);
                    /*Calculate the addition between the two multiplication results calculating by the upper two functions.*/
                    IsOverFlow = TN_Cal64Add(&Product64H_32,&Product64L_32,Result);
                }
            }
            /*The U64Factor is zero when it is overflow.*/
            if(1 == IsOverFlow) {
                Result->High32 = 0U;
                Result->Low32 = 0U;
            }
        }
    }
    return IsOverFlow;
}

/**
* @brief Calculate the remainder with 64 bits.
* @param MulRes             [IN]The multiplication result.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param Dividend           [IN]The dividend.
*                               - Type: U64_VALUE_t *\n
*                               - Range: N/A.\n
*
* @return The remainder
*                               - Type: U64_VALUE_t \n
*                               - Range: N/A.\n
*
*/
__attribute__((section(".ram_code"))) static U64_VALUE_t TN_CalRemaider(const U64_VALUE_t MulRes,const U64_VALUE_t Dividend)
{
    U64_VALUE_t Remaider = {0};                  /*The division remainder with 64 bit.*/
    if( MulRes.Low32 <= Dividend.Low32)/*Do not need to carry calculation*/
    {
        Remaider.Low32 = Dividend.Low32 - MulRes.Low32;
        Remaider.High32 = Dividend.High32 - MulRes.High32;
    }
    else    /*Need to carry calculation*/
    {
        Remaider.Low32 = MAXVALUEOF32BITS - MulRes.Low32 + 1U + Dividend.Low32;
        Remaider.High32 = Dividend.High32 - MulRes.High32 - 1U;
    }
    return Remaider;
}

/**
* @brief Two numbers with 64 bit are divided by 2 respectively until the high 32 bit of the second number is zero.
* @param Number1             [IN]A pointer to the first number with 64 bit.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param Number2             [IN]A pointer to the second number with 64 bit.
*                               - Type: unsigned char *\n
*                               - Range: N/A.\n
* @param Number1Div          [OUT]A pointer to the first divided number which has been divided by 2.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param Number2Div          [OUT]A pointer to the second number which has been divided by 2.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
*
* @return none
*
*
*/
__attribute__((section(".ram_code"))) static void TN_Div2NumBy2(
    const U64_VALUE_t * const Number1,    /*A pointer to the first number with 64 bit.*/
    const U64_VALUE_t * const Number2,    /*A pointer to the second number with 64 bit.*/
    U64_VALUE_t * const Number1Div,       /*A pointer to the first number which has been divided by 2.*/
    U64_VALUE_t * const Number2Div)       /*A pointer to the second number which has been divided by 2.*/
{
    if ((0 != Number1Div) && (0 != Number2Div) && (0 != Number1) && (0 != Number2)) {/*Check the output buffer.*/
        /*Initiate the output buffer.*/
        Number1Div->Low32 = Number1->Low32;
        Number1Div->High32 = Number1->High32;
        Number2Div->Low32 = Number2->Low32;
        Number2Div->High32 = Number2->High32;
        while (0U != Number2Div->High32) {
                /*The high 32 bit and low 32 bit of the first number are divided by 2.*/
                Number1Div->Low32 >>= 1U;
                if ((Number1Div->High32 % 2U) > 0U) {
                    Number1Div->Low32 += 0x80000000U;
                }
                Number1Div->High32 >>= 1U;
                /*The high 32 bit and low 32 bit of the second number are divided by 2.*/
                Number2Div->Low32 >>= 1U;
                if((Number2Div->High32 %2U) > 0U) {
                    Number2Div->Low32 += 0x80000000U;
                }
                Number2Div->High32 >>= 1U;
        }
    }
}

/**
* @brief Calculate the quotient between the 64 bit dividend and 32 bit divisor. The value of the dividend and divisor are treated as unsigned interger.
*        The division used in this function is the standard algorithm used for pen-and-paper division of multidigit numbers expressed in decimal notation.
* @param U64Dividend         [IN]The dividend with 64 bit.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param U32Divisor          [IN]The divisor with 32 bit.
*                               - Type: unsigned int \n
*                               - Range: N/A.\n
*
* @return none
*
*/
__attribute__((section(".ram_code"))) static unsigned int TN_CalQuto(
    const U64_VALUE_t * const U64Dividend,    /*The dividend with 64 bit.*/
    const unsigned int U32Divisor)                 /*The divisor with 32 bit.*/
{
    unsigned int i = 0U; /*The index for the FOR loop.*/
    unsigned int l_uFlag=0U;  /*The most significant bit of the dividend which has been shifted.*/
    U64_VALUE_t l_DivdShift={0};   /*The dividend which has been shifted.*/
    unsigned int l_Quto = 0U;         /*The quotient.*/
    if (0 != U64Dividend) {/*Check the input buffer.*/
        l_DivdShift.High32 = U64Dividend->High32;
        l_DivdShift.Low32 = U64Dividend->Low32;
        for (i = 0U; i < 32U; i++) {
            l_uFlag = (unsigned int)l_DivdShift.High32 >> 31U;    /*The 'uFlag' is 0xffffffff when the most significant bit of the 'l_DivdShift.High32' is one, otherwise 0x00000000.*/
            /*Left-shift dividend by 1 bit.*/
            l_DivdShift.High32 = (l_DivdShift.High32 << 1)|(l_DivdShift.Low32 >> 31);
            l_DivdShift.Low32 <<= 1;
            /*Left-shift quotient by 1 bit.*/
            l_Quto = l_Quto<<1;
            if((l_DivdShift.High32 >= U32Divisor) || (l_uFlag >0U)) { /*Check whether the dividend high 32 bit is greater than the divisor low 32 bit after left shifting.*/
                /*Calculate the quotient and remainder between the dividend high 32 bit and divisor low 32 bit.*/
                l_DivdShift.High32 -= U32Divisor;
                l_Quto++;
            }
        }
    }
    return l_Quto;
}
/**
* @brief Check the validation of the function input and output.
* @param i_pDividend            [IN]A pointer to the dividend with 64 bit.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param i_pDivisor             [IN]A pointer to the divisor with 64 bit.
*                               - Type: U64_VALUE_t *\n
*                               - Range: N/A.\n
* @param o_pQuto                [OUT]A pointer to the division quotient with 32 bit.
*                               - Type: unsigned int  *\n
*                               - Range: N/A.\n
* @param ErrorCode           [OUT]A pointer to the division error code.
*                               - Type: unsigned char  *\n
*                               - Range: N/A.\n
* @param DivStatus           [OUT]A pointer to the division successful status.
*                               - Type: unsigned char  *\n
*                               - Range: N/A.\n
*
* @return none
*
*/
__attribute__((section(".ram_code"))) static void TN_CheckUINT64AccurateDiv(
    const U64_VALUE_t * const i_pDividend,               /*A pointer to the dividend with 64 bit.*/
    const U64_VALUE_t * const i_pDivisor,                /*A pointer to the divisor with 64 bit.*/
    unsigned int * const o_pQuto,                             /*A pointer to the division quotient with 32 bit.*/
    unsigned char * const ErrorCode,    /*A pointer to the division error code.*/
    unsigned char * const o_pDivStatus)                     /*A pointer to the division successful status.*/
{
    if ((0 != o_pQuto) && (0 != ErrorCode) && (0 != o_pDivStatus)) { /*Check the output buffer.*/
        /*Initiate the output buffer.*/
        *ErrorCode = 0;
        *o_pQuto = 0U;
        *o_pDivStatus = 1;
        if ((0 == i_pDividend) || (0 == i_pDivisor)) { /*Check the input buffer.*/
            *ErrorCode = 0xff;
            *o_pQuto = 0U;
            *o_pDivStatus = 0;
        }
    }
}
/**
* @brief Calculate the 64 bit accurate division, and the quotient is rounded.
* @param i_pDividend            [IN]A pointer to the dividend with 64 bit.
*                               - Type: U64_VALUE_t  *\n
*                               - Range: N/A.\n
* @param i_pDivisor             [IN]A pointer to the divisor with 64 bit.
*                               - Type: U64_VALUE_t *\n
*                               - Range: N/A.\n
* @param o_pQuto                [OUT]A pointer to the division quotient with 32 bit.
*                               - Type: unsigned int  *\n
*                               - Range: N/A.\n
* @param ErrorCode           [OUT]A pointer to the division error code.
*                               - Type: unsigned char  *\n
*                               - Range: N/A.\n
* @param o_pDivStatus           [OUT]A pointer to the division successful status.
*                               - Type: unsigned char  *\n
*                               - Range: N/A.\n
*
* @return N/A
*
*/
__attribute__((section(".ram_code"))) static void TN_U64AccurateDiv(
    const U64_VALUE_t * const Dividend,               /*A pointer to the dividend with 64 bit.*/
    const U64_VALUE_t * const Divisor,                /*A pointer to the divisor with 64 bit.*/
    unsigned int * const Quto, 
    U64_VALUE_t * const Remainder,                            /*A pointer to the division quotient with 32 bit.*/
    unsigned char * const ErrorCode,    /*A pointer to the division error code.*/
    unsigned char * const DivStatus)                      /*A pointer to the division successful status.*/
{
    U64_VALUE_t l_DividendDiv2 = {0};        /*The temporary buffer.*/
    U64_VALUE_t l_DivisorDiv2 = {0};         /*The temporary buffer.*/
    unsigned int l_QutoTemp = 0U;             /*The quotient has not been rounded.*/
    TN_CheckUINT64AccurateDiv(Dividend, Divisor, Quto, ErrorCode, DivStatus);
    if (0 == *ErrorCode) {
        if ((0U == Divisor->High32) && (0U == Divisor->Low32)) {/*Check whether the divisor is zero.*/
            /*The division result is zero and division status is not successful when the divisor is zero.*/
            *Quto = 0;
            *ErrorCode  = 1;
            *DivStatus =  0;
        }
        else {
            if ((0U == Divisor->High32) && (Dividend->High32 >= Divisor->Low32)) {/*Check whether the dividend high 32 bit is greater than the divisor low 32 bit.*/
                /*The division result is overflow when the dividend high 32 bit is greater than the divisor low 32 bit.*/
                *Quto = 0;
                *ErrorCode = 2;
                *DivStatus =  0;
            }
            else {
                if ((0U == Divisor->High32) && (0U == Dividend->High32)) { /*Check whether the high 32 bit of the divisor and dividend is zero.*/
                    /*Get the division result directly the high 32 bit of the divisor and dividend is zero.*/
                    l_QutoTemp = Dividend->Low32 / Divisor->Low32;
                }
                else {
                    /*The dividend and divisor are divided by 2 respectively until the high 32 bit of the divisor is zero.*/
                    TN_Div2NumBy2(Dividend,Divisor,& l_DividendDiv2,& l_DivisorDiv2);
                    /*Calculate the quotient and remainder of the 64 bit division by subtracting with borrow.*/
                    l_QutoTemp = TN_CalQuto(&l_DividendDiv2,l_DivisorDiv2.Low32);
                }

                U64_VALUE_t U64MulRes = {0};
                TN_UINT64Multiply(Divisor, l_QutoTemp, &U64MulRes);/*Calculate the multiplication result between the 'Divisor' and 'QutoNotRound'.*/
                /*Calculate the remainder.*/
                *Remainder = TN_CalRemaider(U64MulRes, *Dividend);
                *Quto = l_QutoTemp;
            }
        }
    }
}

/**
 * @brief This function makes the MCU actually enter low power mode.
 * @param[in]   DeepSleep set whether to enter deep sleep mode(1: enter deep sleep, 0: enter suspend)
 * @param[in]   WakeupSrc set wakeup source
 * @param[in]   SleepDurationUs set the duration time of low power
 * @return none
 */
__attribute__((section(".ram_code"))) unsigned char PM_LowPwrEnter2(int DeepSleep, int WakeupSrc, unsigned long SleepDurationUs)
{
    unsigned int t0 = ClockTime();
    int system_tick_enable = WakeupSrc & WAKEUP_SRC_TIMER;
    unsigned char i = 0;
    unsigned char qdec_wakeup_en = 0;
    unsigned int tick_cur_32k;
    unsigned int tick_cur;
    unsigned short cali_32k_cnt = REG_PM_SYSTIMER_32K_CAL_CNT;

    if (system_tick_enable) {
        if (SleepDurationUs < PM_SLEEP_DURATION_MIN) {
            //if duration of sleep is too short, do empty loop instead of actual sleep
            WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f); //clear all status
            unsigned char st;
            do {
                st = ReadAnalogReg (ANA_REG_PM_WAKEUP_STATUS) & 0x0f;
            } while (((unsigned int)(ClockTime() - t0) < (SleepDurationUs*TickPerUs)) && !st);
            return st;
        }
    } 

    unsigned char r = REG_PM_IRQ_EN; //irq disable
    REG_PM_IRQ_EN = 0;

    /******set wakeup source ******/
    WriteAnalogReg(ANA_REG_PM_WAKEUP_SOURCE, (WakeupSrc & 0xff));

    unsigned char anareg2c = 0;        //power down control
    unsigned char anareg2d = ReadAnalogReg(ANA_REG_PM_XTAL_CTRL);        //power down control

    REG_PM_WAKEUP_DIG_SRC = 0x00;
    if (WakeupSrc >> 8) {                               //usb/digital_gpio/qdec wakeup
        REG_PM_WAKEUP_DIG_SRC = (WakeupSrc >> 8);
        if ((WakeupSrc>>8) & 0x10) {                    //if enable qdec wankeup, set 0xd4[0] to 1
            REG_PM_WAKEUP_QDEC_EN |= BIT(0);
            qdec_wakeup_en = 1;
        }
    }
    WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f);                //clear all flag
    
    /****** set power down ******/
    if (system_tick_enable) {
        anareg2c &= 0xfc; //32K RC need be enabled
        // anareg2c &= 0xfe;
    }
    else {
        REG_PM_SYSTIMER_CTRL = 0x20;
        anareg2c |= BIT(0);
        anareg2c |= BIT(1);
    }

    if (qdec_wakeup_en) {                       //32K RC need be enabled
        anareg2c &= 0xfc;
        // anareg2c &= 0xfe;
    }

    /****** change to 32M RC clock before suspend ******/
    unsigned char reg66 = REG_PM_CLOCK_SEL;
    REG_PM_CLOCK_SEL = 0;

    WriteAnalogReg(ANA_REG_PM_PWDOWN_MASK, (DeepSleep ? 0xfc : 0x5e) | anareg2c);
    // WriteAnalogReg(0x2c, (DeepSleep ? 0xfe : 0x5e) | anareg2c);
    WriteAnalogReg(ANA_REG_PM_XTAL_CTRL, anareg2d & 0xdf); 

    //set DC rst duration
    unsigned int span = PM_DC_RESET_DURATION*TickPerUs;
    unsigned int M = span / cali_32k_cnt;
    unsigned int m = span % cali_32k_cnt;
    span = M*16 + m/(cali_32k_cnt/16);
    unsigned char rst_cycle =  0xff - span;
    WriteAnalogReg(ANA_REG_PM_DCDC_DELAY_TIME, SWAP_BIT0_BIT6(rst_cycle));

    //set 32k wakeup tick
    U64_VALUE_t TmpVal[4] = {
        {0, 0},
        {0, 0},
        {0, 0},
        {0, 0}
    };
    unsigned char ErrorCode = 0;
    unsigned char DivStatus = 0;
    //1. calculate the absolute System wakeup tick
    TmpVal[0].Low32 = SleepDurationUs;
    TN_UINT64Multiply(&TmpVal[0], TickPerUs, &TmpVal[1]);
    TmpVal[2].Low32 = t0;
    TN_Cal64Add(&TmpVal[1], &TmpVal[2], &TmpVal[3]);
    unsigned int WakeupTick = TmpVal[3].Low32;
    TmpVal[0].Low32 = 0;
    TmpVal[0].High32 = 0;
    TmpVal[1].Low32 = 0;
    TmpVal[1].High32 = 0;
    TmpVal[2].Low32 = 0;
    TmpVal[2].High32 = 0;
    TmpVal[3].Low32 = 0;
    TmpVal[3].High32 = 0;

    //2. calculate and set the 32K timer wakeup tick
    TmpVal[2].Low32 = cali_32k_cnt;
    tick_cur_32k = PM_Get32kTick();
    tick_cur = ClockTime();
    TmpVal[0].Low32 = SleepDurationUs - (((unsigned int)(tick_cur - t0)) / TickPerUs) - PM_WAKE_UP_MARGIN;
    TN_UINT64Multiply(&TmpVal[0], TickPerUs, &TmpVal[1]);
    TN_U64AccurateDiv(&TmpVal[1], &TmpVal[2], &M, &TmpVal[3], &ErrorCode, &DivStatus);
    m = M*16 + (TmpVal[3].Low32)/(cali_32k_cnt/16);
    REG_PM_SYSTIMER_CTRL = 0x2c;
    REG_PM_SYSTIMER_32K_TICK = (tick_cur_32k + m);
    REG_PM_SYSTIMER_TRIG = BIT(3);
    while (REG_PM_SYSTIMER_TRIG & BIT(3)); 
    WriteAnalogReg(ANA_REG_PM_WAKEUP_STATUS, 0x0f); 

    pm_start();

    if (DeepSleep) {
        REG_PM_POWER_DOWN_CTRL = 0x20; //reboot
    }

    /*suspend recover setting*/
    WriteAnalogReg(ANA_REG_PM_PWDOWN_MASK, 0x00);

    //recover system clock from 32k clock
    TmpVal[0].Low32 = 0;
    TmpVal[0].High32 = 0;
    TmpVal[1].Low32 = 0;
    TmpVal[1].High32 = 0;
    TmpVal[2].Low32 = 0;
    TmpVal[2].High32 = 0;
    TmpVal[3].Low32 = 0;
    TmpVal[3].High32 = 0;
    span = (unsigned int)(PM_Get32kTick() - tick_cur_32k);
    M = span / 16;
    m = span % 16;
    TmpVal[0].Low32 = M;
    TN_UINT64Multiply(&TmpVal[0], cali_32k_cnt, &TmpVal[1]);
    TmpVal[0].Low32 = m*cali_32k_cnt/16;
    TN_Cal64Add(&TmpVal[1], &TmpVal[0], &TmpVal[2]);
    TmpVal[3].Low32 = tick_cur;
    TmpVal[0].Low32 = 0;
    TmpVal[0].High32 = 0;
    TN_Cal64Add(&TmpVal[2], &TmpVal[3], &TmpVal[0]);
    tick_cur = TmpVal[0].Low32;

    /***clear "qdec_irq" irq source flag***/
    if (REG_PM_WAKEUP_QDEC_IRQ & 0x01) {  
        REG_PM_WAKEUP_QDEC_IRQ |= 0x01;
        while(REG_PM_WAKEUP_QDEC_IRQ & 0x01);
    }

    /******system clock has to be on******/
    REG_PM_CLOCK_SEL = reg66; //restore system clock
    REG_PM_SYSTIMER_TICK = tick_cur; //recover system tick
    REG_PM_SYSTIMER_CTRL = 0x90; //auto mode and enable 32K rc calibration
    REG_PM_SYSTIMER_TRIG = 0x01; //enable system timer

    unsigned char anareg44 = ReadAnalogReg(ANA_REG_PM_WAKEUP_STATUS);
    if (anareg44 & BIT(1)) { //wakeup from timer
        while ((unsigned int)(ClockTime() - WakeupTick) > BIT(30));
    }

    REG_PM_IRQ_EN = r; //restore the irq
    return anareg44;
}

#if 0
//for test
U64_VALUE_t TestDividend[3] = {
    {0x00000011, 0x11223345},
    {0x000011cc, 0x33223344},
    {0x001122cc, 0x33223344}
};

U64_VALUE_t TestDivisor[3] = {
    {0x00000011, 0x11223344},
    {0x00000000, 0x33223344},
    {0x00000011, 0x55223344}
};

U64_VALUE_t TestFactor1[3] = {
    {0x00000011, 0x11223344},
    {0x00000000, 0x33223344},
    {0x00001122, 0x55667788}
};

unsigned int TestFactor2[3] = {
    0x00000011,
    0x00001122,
    0x00002233,
};

U64_VALUE_t TestResult = {0, 0};
U64_VALUE_t TestRemainder = {0, 0};
unsigned int TestResult2 = 0;

extern void *  memset(void * d, int c, unsigned int  n);
void U64OpeTest(void)
{
    unsigned char ErrorCode = 0;
    unsigned char DivStatus = 0;
    int i = 0;
    for (i = 0; i < 3; ++i) {
        memset(&TestResult, 0, sizeof(TestResult));
        TN_UINT64Multiply(&TestFactor1[i], TestFactor2[i], &TestResult);
        LogMsg("TN_UINT64Multiply:", &TestResult, sizeof(TestResult));
        WaitMs(1000);
    }

    for (i = 0; i < 3; ++i) {
        memset(&TestResult2, 0, sizeof(TestResult2));
        memset(&TestRemainder, 0, sizeof(TestRemainder));
        TN_U64AccurateDiv(&TestDividend[i], &TestDivisor[i], &TestResult2, &TestRemainder, &ErrorCode, &DivStatus);
        LogMsg("U64Div Quto:", &TestResult2, sizeof(TestResult2));
        LogMsg("U64Div Remainder:", &TestRemainder, sizeof(TestRemainder));
        WaitMs(1000);
    }
}
#endif
