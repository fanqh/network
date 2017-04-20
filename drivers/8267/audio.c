
/********************************************************************************************************
 * @file     audio.c
 *
 * @brief    This file provides set of driver functions to manage the audio module
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
#include "audio.h"
#include "pga.h"
//set period for Misc
#define    SET_PFM(v)    WRITE_REG16(0x800030,(v<<2)&0x0FFF)
//set period for L
#define    SET_PFL(v)    WRITE_REG8(0x800032,v)


unsigned char audio_mode;
static unsigned char adc_IOPowerSupplySet(unsigned char IOp)
{
    unsigned char vv1;
    if (IOp > 2 || IOp < 1) {
        return 0;
    }
    else {
        vv1 = ReadAnalogReg(0x02);
        vv1 = vv1 & 0xcf;
        vv1 = vv1 | (IOp << 4);
        WriteAnalogReg(0x02, vv1);
        return 1;
    }
}

/**
 * @brief audio init function, call the adc init function, configure ADC, PGA and filter parameters used for audio sample
 *        and process
 * @param[in]   mFlag audio input mode flag, '1' differ mode; '0' single end mode
 * @param[in]   bcm_inputCh battery check mode and input channel selection byte, the largest bit indicates mode, the lower
 *              7 bits indicates input channel
 * @param[in]   audio_channel enum variable, indicates the audio input source Analog MIC or Digital MIC
 * @param[in]   adc_max_m Misc period set parameter, T_Misc = 2 * adc_max_m
 * @param[in]   adc_max_l Left channel period set, T_Left = 16*adc_max_l
 * @param[in]   d_samp - decimation filter down sample rate
 * @return none
 */
void Audio_Init(unsigned char mFlag, unsigned char checkM, enum AUDIOINPUTCH audio_channel, unsigned short adc_max_m, unsigned char adc_max_l, enum AUDIODSR d_samp)
{
    unsigned char tem, fhsBL, fhsBH, adc_mod;
    
    
    WRITE_REG8(0x800065,READ_REG8(0x800065)|0x04);//Enable dfifo CLK

    if (audio_channel == AMIC) {
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

        /*******config adc clk as 4MHz*****/
        WRITE_REG8(0x800069, 0x04); //adc clk step
        WRITE_REG8(0x80006a, adc_mod); //adc clk mod
        WriteAnalogReg(0x06, ReadAnalogReg(0x06) & 0xfe); //power on sar
        (*(volatile unsigned char  *)0x80006b) |= 0x80;//Enable adc CLK

        /****************************ADC setting for analog audio sample*************************************/
        /****Set reference voltage for sampling AVDD****/
        *(volatile unsigned char  *)0x80002b &= 0xF3;//Set reference voltage (V_REF)as  RV_AVDD
        // *(volatile unsigned char  *)0x80002b |= 0x04;
        /****signed adc data****/
        *(volatile unsigned char  *)0x80002d |= 0x80;
        /****set adc resolution to 14 bits****/
        *(volatile unsigned char  *)0x80002f &= 0xF8;
        *(volatile unsigned char  *)0x80002f |= 0x07;
        /****set adc sample time to 3 cycles****/
        *(volatile unsigned char  *)0x80003d &= 0xF8;

        if (mFlag) {//diff mode
            //L channel pga-vom input 
            *(volatile unsigned char  *)(0x80002d) &= 0xE0;
            *(volatile unsigned char  *)(0x80002d) |= 0x0D;
            //adc_AnaChSet(LCHANNEL,PGAVOM);
            //adc_AnaModeSet(LCHANNEL,PGAVOPM);
            *(volatile unsigned char  *)(0x80002d) &= 0x9F;
            *(volatile unsigned char  *)(0x80002d) |= 0x60;
            audio_mode = 1;//start PGA
        }
        else {//single end mode
            audio_mode = 0;
            //set L channel single end mode
            *(volatile unsigned char  *)(0x80002d) &= 0x1F;
        }

        WRITE_REG8(0x800b03,0x32); //audio input select AMIC, enable dfifo, enable wptr 
        SET_PFM(adc_max_m); //set Misc channel period
        SET_PFL(adc_max_l); //set L channel period
        WRITE_REG8(0x800033,0x25); //stero mode; L channel enable for audio; R channel enable for battery detect
    }
    else if (audio_channel == DMIC) {
        /*******config DMIC clk as 1MHz*****/
        WRITE_REG8(0x80006c,0x01); // dmic clk step as 1
        WRITE_REG8(0x80006d,0xc0); // dmic clk mode as 192
        *(volatile unsigned char  *)0x80006c |= 0x80;//Enable dmic CLK
        WRITE_REG8(0x800b03,0x30);//audio input select DMIC, enable dfifo, enable wptr
        WRITE_REG8(0x800b06,0x40);//enable DMIC input volume auto control
    }

    WRITE_REG8(0x800b04, d_samp); //setting down sample rate
    
    /********************************************Filter setting*****************************/
    // WRITE_REG8(0x800b04,0x21);// reg0xb04[6:4] cic filter output select 1 , cic[22:5] , reg0xb04[3:0] down scale by 2,


    tem = READ_REG8(0x800b05);
    tem = tem & 0xF0;
    tem = tem | 0x0B;
    WRITE_REG8(0x800b05,tem);//hpf 11
    WRITE_REG8(0x800b06,0x24);//ALC Volume setting

	// Audio_BatteryCheckInit(checkM);
}

/**
 * @brief tune the audio sample rate
 * @param[in]   fine_tune tuing step
 * @return none
 */
void Audio_FineTuneSampleRate(unsigned char fine_tune)
{
    unsigned char tmp = READ_REG8(0x800030);
    tmp |= (fine_tune & 0x03);

    WRITE_REG8(0x800030, tmp);
}



/**
 * @brief sdm set function, enabl or disable the sdm output, configure SDM output paramaters
 * @param[in]   audio_out_en audio output enable or disable set, '1' enable audio output; '0' disable output
 * @param[in]   sdm_setp SDM clk divider
 * @param[in]   sdm_clk SDM clk, default to be 8Mhz
 * @return none
 */
void Audio_SDMOutputSet(unsigned char audio_out_en,unsigned short sdm_step,unsigned char sdm_clk)
{
    unsigned char tem;
    if (audio_out_en) {
        /***1.Set SDM output clock to 8 Mhz***/
        WRITE_REG8(0x800067,(0x80|sdm_clk));//i2s clock, 8M Hz
        WRITE_REG8(0x800068,0xc0);//mod_i = 192MHz
        WRITE_REG16(0x800564,sdm_step);

        /***2.Enable PN generator as dither control, clear bit 2, 3, 6***/
        CLRB(0x800560,0x4C);
        SETB(0x800560,0x30);//set bit 4 bit 5
        CLRB(0x800560,0x03);//Close audio output
        WRITE_REG8(0x800562, 0x08);//PN generator 1 bits ussed
        WRITE_REG8(0x800563, 0x08);//PN generator 2 bits ussed

        /***3.Enable SDM pins(sdm_n, sdm_p)***/
        tem = READ_REG8(0x8005a6);
        tem = tem &0xFC;
        WRITE_REG8(0x8005a6,tem);//pe[1:0] gpio off
        tem = READ_REG8(0x8005b4);
        tem = tem &0xFC;
        WRITE_REG8(0x8005b4,tem);//pe[1:0] as sdm_n, sdm_p

        /***4.enable audio, enable sdm player***/
        SETB(0x800560,0x03);
    }
    else
        CLRB(0x800560,0x02);

}



/**
 * @brief audio input set function, select analog audio input channel, start the filters
 * @param[in]   adc_ch if audio input as signle end mode, should identify an analog audio signal input channel, 
 *              enum variable of ADCINPUTCH
 * @return none
 */
void Audio_InputSet(unsigned char adc_ch)
{
    unsigned char tem;
    if (audio_mode) {
        //PGA Setting
        pgaInit();
        preGainAdjust(DB20);//set pre pga gain to 0
        postGainAdjust(DB9);//set post pga gain to 0
    }
    else {
        //L channel's input as C[0]
        //adc_AnaChSet(LCHANNEL,adc_ch);
        *(volatile unsigned char  *)(0x80002d) &= 0xE0;
        *(volatile unsigned char  *)(0x80002d) |= adc_ch;
        }
    //open lpf, hpf, alc
    tem = READ_REG8(0x800b05);
    tem = tem & 0x8F;
    WRITE_REG8(0x800b05,tem);//lpf, hpf, alc on
}


/**
 * @brief set audio volume level
 * @param[in]   input_output_select select the tune channel, '1' tune ALC volume; '0' tune SDM output volume
 * @param[in]   volume_set_value volume level
 * @return none
 */
void Audio_VolumeSet(unsigned char input_output_select,unsigned char volume_set_value)
{
    if (input_output_select)
        WRITE_REG8(0x800b06,volume_set_value);
    else
        WRITE_REG8(0x800561,volume_set_value);
}


/*************************************************************
*  
*  @brief  automatically gradual change volume 
*
*  @param  vol_step - volume change step, the high part is decrease step while the low part is increase step
*      gradual_interval - volume increase interval
*
*  @return  none
*/
void Audio_VolumeStepChange(unsigned char vol_step,unsigned short gradual_interval)
{
    // unsigned char low_part,high_part;
    // low_part = (unsigned char)gradual_interval;
    // high_part = (unsigned char)(gradual_interval>>8);
    WRITE_REG8(0x800b0b,vol_step);
    WRITE_REG16(0x800b0c,gradual_interval);
    // WRITE_REG8(0x800b0d,high_part);
}


/**
 * @brief get the battery value
 * @param   none
 * @return the sampled value, 7 bits resolution
 */
#define    CHECKADCSTATUS    (((*(volatile unsigned char  *)0x80003a) & 0x01) ? 1:0)
#if 1 // diff mode on off, audio on
unsigned short Audio_BatteryValueGet(void)
{
    unsigned short sampledValue;
    
    while(CHECKADCSTATUS);
    sampledValue = READ_REG16(0x800038);
    sampledValue = (sampledValue&0x3F80);
    sampledValue = (sampledValue -128)*64*1428/63/16384; //output the detected voltage in Unit "mv" of reference 1.428V
    return sampledValue;
}

#else //for diff mode on
    unsigned short Audio_BatteryValueGet(void){
    unsigned short sampledValue;
    while(CHECKADCSTATUS);
    sampledValue = READ_REG16(0x800038);
    sampledValue = sampledValue&0x3F80;
    return (sampledValue>>7);
}
#endif

