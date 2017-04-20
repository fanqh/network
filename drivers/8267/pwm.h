/********************************************************************************************************
 * @file     pwm.h
 *
 * @brief    This file provides set of functions for PWM module
 *
 * @author   junjun.xu@telink-semi.com; qiuwei.chen@telink-semi.com
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
#ifndef 	pwm_H
#define 	pwm_H

/** define the macro that configures pin port for PWM module */
#define    PWM0_CFG_GPIO_A0()    do{\
									*(volatile unsigned char  *)0x800586 &= 0xfe;\
									*(volatile unsigned char  *)0x8005b0 &= 0xfe;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */
#define    PWM0_CFG_GPIO_C0()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xfe;\
									*(volatile unsigned char  *)0x8005b2 &= 0xfe;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM0_CFG_GPIO_D5()    do{\
									*(volatile unsigned char  *)0x80059e &= 0xdf;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM0_CFG_GPIO_E0()    do{\
									*(volatile unsigned char  *)0x8005a6 &= 0xfe;\
									*(volatile unsigned char  *)0x8005b4 |= 0x01;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM1_CFG_GPIO_A3()    do{\
									*(volatile unsigned char  *)0x800586 &= 0xf7;\
									*(volatile unsigned char  *)0x8005b0 &= 0xfb;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM1_CFG_GPIO_C1()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xfd;\
									*(volatile unsigned char  *)0x8005b2 &= 0xfd;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM1_CFG_GPIO_D6()    do{\
									*(volatile unsigned char  *)0x80059e &= 0xbf;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM1_CFG_GPIO_E1()    do{\
									*(volatile unsigned char  *)0x8005a6 &= 0xfd;\
									*(volatile unsigned char  *)0x8005b4 |= 0x02;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM2_CFG_GPIO_B0()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xfe;\
									*(volatile unsigned char  *)0x8005b1 |= 0x01;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM2_CFG_GPIO_C2()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xfb;\
									*(volatile unsigned char  *)0x8005b2 &= 0xfb;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM2_CFG_GPIO_D7()    do{\
									*(volatile unsigned char  *)0x80059e &= 0x7f;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM3_CFG_GPIO_B2()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xfb;\
									*(volatile unsigned char  *)0x8005b1 &= 0xfb;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM3_CFG_GPIO_C3()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xf7;\
									*(volatile unsigned char  *)0x8005b2 &= 0xf7;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM4_CFG_GPIO_B4()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xef;\
									*(volatile unsigned char  *)0x8005b1 &= 0xef;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM4_CFG_GPIO_C4()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xef;\
									*(volatile unsigned char  *)0x8005b2 &= 0xef;\
								 }while(0)

/** define the macro that configures pin port for PWM module */ 
#define    PWM5_CFG_GPIO_B6()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xbf;\
									*(volatile unsigned char  *)0x8005b1 &= 0xbf;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM5_CFG_GPIO_C5()    do{\
									*(volatile unsigned char  *)0x800596 &= 0xdf;\
									*(volatile unsigned char  *)0x8005b2 &= 0xdf;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM0N_CFG_GPIO_A2()   do{\
									*(volatile unsigned char  *)0x800586 &= 0xfb;\
									*(volatile unsigned char  *)0x8005b0 &= 0xfd;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM1N_CFG_GPIO_A4()   do{\
									*(volatile unsigned char  *)0x800586 &= 0xef;\
									*(volatile unsigned char  *)0x8005b0 &= 0xef;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM2N_CFG_GPIO_A5()   do{\
									*(volatile unsigned char  *)0x800586 &= 0xdf;\
									*(volatile unsigned char  *)0x8005b0 &= 0xdf;\
								 }while(0) 

/** define the macro that configures pin port for PWM module */ 
#define    PWM2N_CFG_GPIO_B1()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xfd;\
									*(volatile unsigned char  *)0x8005b1 &= 0xfd;\
								 }while(0)

/** define the macro that configures pin port for PWM module */ 
#define    PWM3N_CFG_GPIO_B3()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xf7;\
									*(volatile unsigned char  *)0x8005b1 &= 0xf7;\
								 }while(0)

/** define the macro that configures pin port for PWM module */ 
#define    PWM4N_CFG_GPIO_B5()    do{\
									*(volatile unsigned char  *)0x80058e &= 0xdf;\
									*(volatile unsigned char  *)0x8005b1 &= 0xdf;\
								 }while(0)

/** define the macro that configures pin port for PWM module */ 
#define    PWM5N_CFG_GPIO_B7()    do{\
									*(volatile unsigned char  *)0x80058e &= 0x7f;\
									*(volatile unsigned char  *)0x8005b1 &= 0x7f;\
								 }while(0)



/**
 *  @brief  Define number of PWM modules
 */
enum PWMN{
	PWM0 = 0,
	PWM1,
	PWM2,
	PWM3,
	PWM4,
	PWM5,
};

/**
 *  @brief  Define working modes of PWM modules
 */
enum PWMWM{
	NORMAL,
	COUNT,
	IR = 0x03,
};

/**
 *  @brief  Define irq sources of PWM module
 */
enum PWMIRQSOURCE{
	NOIRQ = 0xff,
	PWM0PNUM = 0,
	PWM1PNUM,
	PWM0CYCLEDONE,
	PWM1CYCLEDONE,
	PWM2CYCLEDONE,
	PWM3CYCLEDONE,
	PWM4CYCLEDONE,
	PWM5CYCLEDONE,
	PWMIRQSOURCENUM,
};

/**
 * @brief This function initiates the PWM module by seting and enabling the PWM clock. 
 * @param[in]   pwmCLKdiv the division factor of PWM clock, i.e., 
 *              PWM clock = System clock / (pwmCLKdiv + 1)
 * @return none
 */
extern void pwm_Init(unsigned char pwmCLKdiv);

/**
 * @brief This function opens a pwm channel and sets the parameters
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @param[in]   pwmWorkM enum variable of pwm work mode
 * @param[in]   phaseTime delay time of the pwm channel before enter into the count status
 * @param[in]   cscValue count status value
 * @param[in]   cycVlaue cycle value
 * @param[in]   sfValue signal frame value for the COUNT and IR mode
 * @return '1' set success; '0' set error, if you set pwm2 - pwm5 to work on the other not normal mode, the function will return error.
 */
extern unsigned char pwm_Open(enum PWMN pwmNumber, enum PWMWM pwmWorkM, unsigned short phaseTime, unsigned short cscValue, unsigned short cycValue, unsigned short sfValue );

/**
 * @brief This function closes a pwm channel and resets the settings to default
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
extern void pwm_Close(enum PWMN pwmNumber);

/**
 * @brief This function starts a pwm channel and enable the output
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
extern void pwm_Start(enum PWMN pwmNumber);

/**
 * @brief This function stops a pwm channel and disable the output
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
extern void pwm_Stop(enum PWMN pwmNumber);

/**
 * @brief This function sets the duty cycle of a pwm channel
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @param[in]   cscValue count status value
 * @param[in]   cycVlaue cycle value
 * @return '1' set success; '0' set error, if csValue>cycValue
 */
extern unsigned char pwm_DutyCycleSet(enum PWMN pwmNumber, unsigned short csValue,unsigned short cycValue);

/**
 * @brief This function enables pwm interrupt
 * @param[in]   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
extern unsigned char pwm_InterruptEnable(enum PWMIRQSOURCE pwmIrqSrc);

/**
 * @brief This function disables pwm interrupt
 * @param[in]   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
extern unsigned char pwm_InterruptDisable(enum PWMIRQSOURCE pwmIrqSrc);

/**
 * @brief This function gets the PWM interrupt source
 * @param   none
 * @return enum variable of pwm irq source
 */
extern enum PWMIRQSOURCE pwm_InterruptSourceGet(void );

/**
 * @brief This function clears interrupt status
 * @param   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
extern void pwm_InterruptStatusClr(enum PWMIRQSOURCE irq_src);

/**
 * @brief This function inverts PWM-INV output (would generate the same output waveform as PWMN)
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
extern void pwm_INVInvert(enum PWMN pwmNumber);

#endif
