/********************************************************************************************************
 * @file     adc.h
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
#ifndef 	adc_new_H
#define 	adc_new_H

//ADC reference voltage
enum ADCRFV{
	RV_1P428,		
	RV_AVDD,		
	RV_1P224,		
};
//ADC resolution 
enum ADCRESOLUTION{
	RES7,	
	RES9,
	RES10,
	RES11,
	RES12,
	RES13,
	RES14,
};

//ADC Sampling time
enum ADCST{
	S_3,			
	S_6,
	S_9,
	S_12,
	S_18,
	S_24,
	S_48,
	S_144,
};

//ADC analog input channel selection enum
enum ADCINPUTCH{
	NOINPUT,
	C0,
	C1,
	C6,
	C7,
	B0,
	B1,
	B2,
	B3,
	B4,
	B5,
	B6,
	B7,
	PGAVOM,
	PGAVOP,
	TEMSENSORN,
	TEMSENSORP,
	AVSS,
	OTVDD,//1/3 voltage division detection
};
//ADC channel input mode
enum ADCINPUTMODE{
	SINGLEEND,
	INVERTB_1,
	INVERTB_3,
	PGAVOPM,
};


//set period for Misc
#define		SET_P(v)			write_reg16(0x800030,(v<<2)&0x0FFF)


//Check adc status, busy return 1
#define		CHECKADCSTATUS		(((*(volatile unsigned char  *)0x80003a) & 0x01) ? 1:0)	

/**
 * @brief This function sets ADC resolution for channel Misc
 * @param[in]   adcRes enum variable adc resolution.
 * @return none
 */
extern void adc_ResSet(enum ADCRESOLUTION adcRes);

/**
 * @brief This function sets ADC input channel
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcInCha enum variable of adc input channel.
 * @return none
 */
extern void adc_AnaChSet( enum ADCINPUTCH adcInCha);

/**
 * @brief This function sets ADC input channel mode - signle-end or differential mode
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   inM enum variable of ADCINPUTMODE.
 * @return none
 */
extern void adc_AnaModeSet( enum ADCINPUTMODE inM);

/**
 * @brief This function sets ADC sample time(the number of adc clocks for each sample)
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcST enum variable of adc sample time.
 * @return none
 */
extern void adc_SampleTimeSet(enum ADCST adcST);

/**
 * @brief This function sets ADC reference voltage for the Misc and L channel
 * @param[in]   adcCha enum variable adc channel.
 * @param[in]   adcRF enum variable of adc reference voltage.
 * @return none
 */
extern void adc_RefVoltageSet(enum ADCRFV adcRF);

/**
 * @brief ADC initiate function, set the ADC clock details (3MHz) and start the ADC clock.
 *        ADC clock relys on PLL, if the FHS isn't selected to 192M PLL (probably modified 
 *        by other parts codes), adc initiation function will returns error.
 * @param   none
 * @return '1' set success; '0' set error
 */
extern unsigned char adc_Init(void );

/**
 * @brief Initiate function for the battery check function
 * @param[in]   checkM Battery check mode, '0' for battery dircetly connected to chip,
 *              '1' for battery connected to chip via boost DCDC
 * @return none
 */
extern void adc_BatteryCheckInit(unsigned char checkM);

/**
 * @brief get the battery value
 * @param   None
 * @return the sampling value
 */
extern unsigned short adc_BatteryValueGet(void);

/**
 * @brief Initiate function for the temparture sensor
 * @param   None
 * @return none
 */
extern void adc_TemSensorInit(void);

/**
 * @brief get the temperature sensor sampled value
 * @param   None
 * @return the adc sampled value 14bits significants
 */
extern unsigned short adc_TemValueGet(void);

/**
 * @brief get adc sampled value
 * @param[in]   adc_ch adc channel select, MISC or the LCHANNEL, enum variable
 * @param[in]   sample_mode adc sample mode, '1' manual mode; '0' auto sample mode
 * @return sampled_value, raw data
 */
extern unsigned short adc_SampleValueGet(void);

#endif

