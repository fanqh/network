/********************************************************************************************************
 * @file     adc.c
 *
 * @brief    This file provides set of driver functions to manage the adc module
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
#include "bsp.h"
#include "adc.h"


#define    EN_ADCCLK    (*(volatile unsigned char  *)0x80006b |= 0x80)

//Select ADC mannul mode
#define    EN_MANUALM    WRITE_REG8(0x800033,0x00)


//Start sampling and conversion process for mannual mode
#define    STARTSAMPLING    WRITE_REG8(0x800035,0x80)

//Read sampling data
#define    READOUTPUTDATA    READ_REG16(0x800038)

/**
 * @brief This function sets ADC reference voltage for the Misc and L channel
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcRF enum variable of adc reference voltage.
 * @return none
 */
void adc_RefVoltageSet(enum ADCRFV adcRF)
{
    unsigned char st;
    
    st = (unsigned char)adcRF;
    *(volatile unsigned char  *)0x80002b &= 0xFC;
    
    *(volatile unsigned char  *)0x80002b |= st;
}


/**
 * @brief This function sets ADC resolution for channel Misc
 * @param[in]   adcRes enum variable adc resolution.
 * @return none
 */
void adc_ResSet(enum ADCRESOLUTION adcRes)
{
    unsigned char resN;
    resN = (unsigned char )adcRes;
    *(volatile unsigned char  *)0x80003c &= 0xC7;
    *(volatile unsigned char  *)0x80003c |= (resN<<3);
}


/**
 * @brief This function sets ADC sample time(the number of adc clocks for each sample)
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcST enum variable of adc sample time.
 * @return none
 */

void adc_SampleTimeSet( enum ADCST adcST)
{
    unsigned char st;
    st = (unsigned char)adcST;
    
    *(volatile unsigned char  *)(0x80003c) &= 0xF8;
    
    *(volatile unsigned char  *)(0x80003c) |= st;
}


/**
 * @brief This function sets ADC input channel
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcInCha enum variable of adc input channel.
 * @return none
 */
void adc_AnaChSet(enum ADCINPUTCH adcInCha)
{
    unsigned char cnI;

    cnI = (unsigned char)adcInCha;

    *(volatile unsigned char  *)(0x80002c) &= 0xE0;
    *(volatile unsigned char  *)(0x80002c) |= cnI;
}

/***************************************************************************
*
*  @brief  set IO power supply for the 1/3 voltage division detection, there are two input sources of the 
*      IO input battery voltage, one through the VDDH and the other through the  ANA_B<7> pin
*
*  @param  IOp - input power source '1' is the VDDH; '2' is the ANA_B<7>.
*
*  @return  '1' setting success; '0' set error
*/
static unsigned char adc_IOPowerSupplySet(unsigned char IOp)
{
    unsigned char vv1;
    if (IOp > 2 || IOp < 1) {
        return 0;
    }
    else {
        vv1 = ReadAnalogReg(0x02);
        vv1 = vv1 & 0xcf;
        vv1 = vv1 | (IOp<<4);
        WriteAnalogReg(0x02, vv1);
        return 1;
    }
}

/**
 * @brief This function sets ADC input channel mode - signle-end or differential mode
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   inM enum variable of ADCINPUTMODE.
 * @return none
 */
void adc_AnaModeSet( enum ADCINPUTMODE inM)
{
    unsigned char cnM;
    
    cnM = (unsigned char)inM;
    *(volatile unsigned char  *)(0x80002c) &= 0x1F;
    *(volatile unsigned char  *)(0x80002c) |= (cnM<<5);
}

/**
 * @brief ADC initiate function, set the ADC clock details (3MHz) and start the ADC clock.
 *        ADC clock relys on PLL, if the FHS isn't selected to 192M PLL (probably modified 
 *        by other parts codes), adc initiation function will returns error.
 * @param   none
 * @return '1' set success; '0' set error
 */
unsigned char adc_Init(void )
{
    unsigned char fhsBL, fhsBH, adc_mod;
    fhsBH = READ_REG8(0x800070) & 0x01; //0x70[0]
    fhsBL = READ_REG8(0x800066) & 0x80; //0x66[7]
    //FHS select 192MHz PLL
    if ((0 == fhsBL) && (0 == fhsBH)) {
        adc_mod = 192;
        WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0x7f); //power on pll
    }
    //FHS select 32MHz RC
    else if ((0x80 == fhsBL) && (0 == fhsBH)) {
        adc_mod = 32;
        WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xfb); //power on 32M rc
    }
    //FHS select 16MHz pad
    else {
        adc_mod = 16;
        WriteAnalogReg(0x05, ReadAnalogReg(0x05) & 0xf7); //power on 16M pad
    }
    
    /******set adc clk as 3MHz******/
    WRITE_REG8(0x800069, 0x03); //adc clk step
    WRITE_REG8(0x80006a, adc_mod); //adc clk mode
    WriteAnalogReg(0x06, ReadAnalogReg(0x06) & 0xfe); //power on sar

    EN_ADCCLK; //Enable adc CLK
    EN_MANUALM;
    return 1;
}

/**
 * @brief Initiate function for the battery check function
 * @param[in]   checkM Battery check mode, '0' for battery dircetly connected to chip,
 *              '1' for battery connected to chip via boost DCDC
 * @return none
 */
void adc_BatteryCheckInit(unsigned char checkM)
{
    /***1.set adc mode and input***/
    WRITE_REG8(0x80002c,0x12);       //select "1/3 voltage division detection" as single-end input
    
    /***2.set battery check mode***/
    if(!checkM)    
        adc_IOPowerSupplySet(1);
    else    
        adc_IOPowerSupplySet(2);
    
    /***3.set adc reference voltage***/    
    adc_RefVoltageSet(RV_1P428);     //Set reference voltage (V_REF)as  1.428V
    
    /***4.set adc resultion***/
    adc_ResSet(RES14);               //Set adc resolution to 14 bits, bit[14] to bit bit[1]
    
    /***5.set adc sample time***/
    adc_SampleTimeSet(S_3);          //set sample time
    
    /***6.enable manual mode***/
    EN_MANUALM;
}

/**
 * @brief get the battery value
 * @param   None
 * @return the sampling value
 */
unsigned short adc_BatteryValueGet(void)
{
    unsigned short sampledValue;
    
    STARTSAMPLING;
    
    while(CHECKADCSTATUS);
    
    sampledValue = READ_REG16(0x800038);
    sampledValue = (sampledValue&0x3FFF);
    sampledValue = (sampledValue -128)*64*1428/63/16384; //output the detected voltage in Unit "mv" of reference 1.428V
    return 3*sampledValue;
}

/**
 * @brief Initiate function for the temparture sensor
 * @param   None
 * @return none
 */
void adc_TemSensorInit(void)
{
    /***1.set adc mode and input***/
    WRITE_REG8(0x80002c,0x0f);  //select TEMSENSORN as single-end input

    /***2. set adc reference voltage***/    
    adc_RefVoltageSet(RV_AVDD);

    /***3.set adc resultion***/
    adc_ResSet(RES14);

    /***4.set adc sample time***/
    adc_SampleTimeSet(S_3);
    
    /***5.enable manual mode***/
    EN_MANUALM;
}

/**
 * @brief get the temperature sensor sampled value
 * @param   None
 * @return the adc sampled value 14bits significants
 */
unsigned short adc_TemValueGet(void)
{
    unsigned short sampledValue;
    STARTSAMPLING;
    while(CHECKADCSTATUS);
    sampledValue = (unsigned short)(READOUTPUTDATA & 0x3FFF);
    STARTSAMPLING;
    while(CHECKADCSTATUS);
    sampledValue = sampledValue - (unsigned short)(READOUTPUTDATA & 0x3FFF);        
    return sampledValue;
}

/**
 * @brief get adc sampled value
 * @param[in]   adc_ch adc channel select, MISC or the LCHANNEL, enum variable
 * @param[in]   sample_mode adc sample mode, '1' manual mode; '0' auto sample mode
 * @return sampled_value, raw data
 */
unsigned short adc_SampleValueGet(void)
{
    unsigned short sampledValue;

    STARTSAMPLING;

    
    while(CHECKADCSTATUS);
        
    return READOUTPUTDATA;
    
}


