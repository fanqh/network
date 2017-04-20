/********************************************************************************************************
 * @file     pwm.c
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
#include "pwm.h"
#include "bsp.h"

#define    PHASE_TIME_BASE   0x800788
#define    CSC_VALUE_BASE    0x800794
#define    CYC_VALUE_BASE    0x800796
#define    SFN_VALUE_BASE    0x8007ac

//Enable the specified pwm, v is the PWMN
#define    EN_PWM(v)    	 (*(volatile unsigned char  *)0x800780 |= (1<<v))

//Disable the specified pwm, v is the PWMN
#define    DIS_PWM(v)        (*(volatile unsigned char  *)0x800780 &= ~(1<<v))

//Set PWM clock frequency, v is the desired clock value, default to SYS_CLK
#define    SET_PWMCLK(v)     (*(volatile unsigned char  *)0x800781 = v)

//PWM output invert
#define    INVERT_PWM(v)     (*(volatile unsigned char  *)0x800783 |= (1<<v))



//Enable pwm output polarity
#define    EN_PWMPOL(v)      (*(volatile unsigned char  *)0x800785 |= (1<<v))

#define    DIS_PWMPOL(v)     (*(volatile unsigned char  *)0x800785 &= ~(1<<v))

//Enable pwm interrupt, v is the interrupt source 
#define    EN_PWMINT(v)      (*(volatile unsigned char  *)0x8007b0 |= (1<<v))
#define    DIS_PWMINT(v)     (*(volatile unsigned char  *)0x8007b0 &= ~(1<<v))

//Enable PWM interrupt mask
#define    EN_PWMINTM        (*(volatile unsigned char  *)0x800641 |= 0x40)
#define    DIS_PWMINTM       (*(volatile unsigned char  *)0x800641 &= 0xBF)

//Clear interrupt status, v is the interrupt source
#define    CLR_PWMIRQS(v)    (*(volatile unsigned char  *)0x8007b1 |= (1<<v))

//Set Phase time, v is the PWM number, n is the desired value
#define    SET_PWMPT(v,n)    WRITE_REG16((PHASE_TIME_BASE + v*2),n)

//PWM count status cycle value set function, v is the PWM numer, n is the desired value
#define    SET_PWMCSCV(v,n)  WRITE_REG16((CSC_VALUE_BASE + v*4),n)

//PWM cycle value set function, v is the PWM numer, n is the desired value
#define    SET_PWMCYCV(v,n)  WRITE_REG16((CYC_VALUE_BASE + v*4),n)

//Set number of signal frames, v can only be pwm0 and pwm1, n is the setting value
#define    SET_PWMSFN(v,n)   WRITE_REG16((SFN_VALUE_BASE + v*2),n)


//Set pwm mode, v can only be pwm0 and pwm1, n is the mode
#define    SET_PWMMODE(v,n)  do{\
                                    *(volatile unsigned char  *)0x800782 &= ~(n<<(v*2));\
                                    *(volatile unsigned char  *)0x800782 |= (n<<(v*2));\
                               }while(0)

//PWM-INV output invert
#define    INVERT_PWMINV(v)  (*(volatile unsigned char  *)0x800784 |= (1<<v))


/**
 * @brief This function initiates the PWM module by seting and enabling the PWM clock. 
 * @param[in]   pwmCLKdiv the division factor of PWM clock, i.e., 
 *              PWM clock = System clock / (pwmCLKdiv + 1)
 * @return none
 */
void pwm_Init(unsigned char pwmCLKdiv)
{
    SET_PWMCLK(pwmCLKdiv);//PWMCLK = SYSCLK/4
    SETB(0x800064,0x10);//PWM CLK Enable
    CLRB(0x800061,0x10);// disable PWM reset, wakeup 
}

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
unsigned char pwm_Open(enum PWMN pwmNumber,enum PWMWM pwmWorkM,unsigned short phaseTime,unsigned short cscValue,unsigned short cycValue,unsigned short sfValue)
{
    unsigned long setValue = 0;
    unsigned char pwmN,pwmM;
    pwmN = (unsigned char)pwmNumber;
    pwmM = (unsigned char)pwmWorkM;
    if (pwmN>1 && pwmM!=0) {
        return 0;
    }
    SET_PWMPT(pwmN,phaseTime);
    setValue = (unsigned long)cscValue;
    setValue = setValue + ((unsigned long)cycValue<<16);
    WRITE_REG32((CSC_VALUE_BASE + pwmN*4),setValue);
    /***set pwm work mode(only for pwm0/pwm1)***/
    if (pwmN < 2) {
        if (pwmN == 1) {
            WRITE_REG8(0x800782, (READ_REG8(0x800782)&0xf3)+(pwmM<<2));
        }
        else {
            WRITE_REG8(0x800782, (READ_REG8(0x800782)&0xfc)+pwmM);
        }
    }
    
    if(pwmWorkM != NORMAL)
        SET_PWMSFN(pwmN,sfValue);
    
    EN_PWM(pwmN);//Should be here, donot enable the PWM unitil all the settings done
    return 1;
}

/**
 * @brief This function closes a pwm channel and resets the settings to default
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
void pwm_Close(enum PWMN pwmNumber)
{
    unsigned char pwmN;
    pwmN = (unsigned char)pwmNumber;
    SET_PWMPT(pwmN,0);
    WRITE_REG32((CSC_VALUE_BASE + pwmN*4),0);
    SET_PWMMODE(pwmN,0);
    if (pwmN < 2 ) {
        SET_PWMSFN(pwmN,0);
    }
    DIS_PWM(pwmN);
}

/**
 * @brief This function starts a pwm channel and enable the output
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
void pwm_Start(enum PWMN pwmNumber)
{
    unsigned char pwmN = (unsigned char)pwmNumber;
    EN_PWM(pwmN);
}

/**
 * @brief This function stops a pwm channel and disable the output
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
void pwm_Stop(enum PWMN pwmNumber)
{
    unsigned char pwmN = (unsigned char)pwmNumber;
    DIS_PWM(pwmN);
}

/**
 * @brief This function sets the duty cycle of a pwm channel
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @param[in]   cscValue count status value
 * @param[in]   cycVlaue cycle value
 * @return '1' set success; '0' set error, if csValue>cycValue
 */
unsigned char pwm_DutyCycleSet(enum PWMN pwmNumber,unsigned short csValue,unsigned short cycValue)
{
    unsigned char pwmN;
    pwmN = (unsigned char)pwmNumber;
    if (cycValue > csValue) {
        WRITE_REG16((CSC_VALUE_BASE + pwmN*4),csValue);
        WRITE_REG16((CYC_VALUE_BASE + pwmN*4),cycValue);
        return 1;
    }
    return 0;
}

/**
 * @brief This function enables pwm interrupt
 * @param[in]   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
unsigned char pwm_InterruptEnable(enum PWMIRQSOURCE pwmIrqSrc)
{
    EN_PWMINT(pwmIrqSrc);
    EN_PWMINTM;
    *(volatile unsigned char  *)0x800643 |= 0x01;//EN_IRQ
    return 1;
}

/**
 * @brief This function disables pwm interrupt
 * @param[in]   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
unsigned char pwm_InterruptDisable(enum PWMIRQSOURCE pwmIrqSrc)
{
    DIS_PWMINT(pwmIrqSrc);
    return 1;
}

/**
 * @brief This function gets the PWM interrupt source
 * @param   none
 * @return enum variable of pwm irq source
 */
enum PWMIRQSOURCE pwm_InterruptSourceGet(void )
{
    unsigned char IrqStatus = READ_REG8(0x8007b1);
    int i = 0;

    if (IrqStatus == 0) {
        return NOIRQ;
    }
    else {
        for (i=PWM0PNUM; i<PWMIRQSOURCENUM; i++) {
            if ((1<<i) & IrqStatus)
                break;
        }
        return (enum PWMIRQSOURCE)i;
    }
}

/**
 * @brief This function clears interrupt status
 * @param   pwmIrqSrc enum variable of pwm irq source
 * @return none
 */
void pwm_InterruptStatusClr(enum PWMIRQSOURCE irq_src)
{
    unsigned char irqS;
    irqS = (unsigned char)irq_src;
    CLR_PWMIRQS(irqS);
}

/**
 * @brief This function inverts PWM-INV output (would generate the same output waveform as PWMN)
 * @param[in]   pwmNumber enum variable of pwm channel number
 * @return none
 */
void pwm_INVInvert(enum PWMN pwmNumber)
{
    INVERT_PWMINV((unsigned char)pwmNumber);
}



