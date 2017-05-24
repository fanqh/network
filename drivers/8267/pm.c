/*
 * pm1.c
 *
 *  Created on: 2017-5-24
 *      Author: Administrator
 */
#include "pm.h"


#define 		POWER_SAVING_16MPAD_DIGITAL		1   //16MHz clock to digital
	#define RESET_TIME_US	    	  2000
	#define EARLYWAKEUP_TIME_US       2100
	#define EMPTYRUN_TIME_US       	  2400
#define SWAP_BIT0_BIT6(x)     ( ((x)&0xbe) | ( ((x)&0x01)<<6 ) | ( ((x)&0x40)>>6 )  )

const BSP_TblCmdSetTypeDef  tbl_cpu_wakeup_init[] = {
	0x0060, 0x00,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
	0x0061, 0x00,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
	0x0062, 0x00,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
	0x0063, 0xff,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
	0x0064, 0xff,		TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x0065, 0xff,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
//	0x063d, 0x1f,		TCMD_UNDER_BOTH | TCMD_WRITE,	//fast SRB speed, not working with USB MCU mode

//	0x0067, 0x00,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
//	0x0066, 0x00,		TCMD_UNDER_BOTH | TCMD_WRITE,	//open all the clk,disable all the rst
	0x0073, 0x04,		TCMD_UNDER_BOTH | TCMD_WRITE,	//low power divider disable
	0x0620, 0x01,		TCMD_UNDER_BOTH | TCMD_WRITE,
	0x074f, 0x01,		TCMD_UNDER_BOTH | TCMD_WRITE,	//enable system tick

	//reg_gpio_wakeup_irq = FLD_GPIO_CORE_WAKEUP_EN | FLD_GPIO_CORE_INTERRUPT_EN
	0x05b5, 0x0c,		TCMD_UNDER_BOTH | TCMD_WRITE,

#if(POWER_SAVING_16MPAD_DIGITAL)
	0x80, 0x71,  TCMD_UNDER_BOTH | TCMD_WAREG,  //16MHz clock to digital power down
#else
	0x80, 0x61,  TCMD_UNDER_BOTH | TCMD_WAREG,
#endif
	0x20, 0xc1,		TCMD_UNDER_BOTH | TCMD_WAREG,	//wakeup reset time: (0xff - 0xc1)*32 = 2000 us
	0x2d, 0x0f,		TCMD_UNDER_BOTH | TCMD_WAREG,	//quick settle: 200 us

	0x05, 0x62,		TCMD_UNDER_BOTH | TCMD_WAREG,	//turn off crystal pad,		bit7: bbpll		(8267)
	0x88, 0x0f,		TCMD_UNDER_BOTH | TCMD_WAREG,	//bit[1:0]: 192M CLOCK to digital			(8267)
};

void PM_WakeupInit(void)
{
	LoadTblCmdSet (tbl_cpu_wakeup_init, sizeof (tbl_cpu_wakeup_init)/sizeof (BSP_TblCmdSetTypeDef));
}


_attribute_ram_code_ _attribute_no_inline_ void  sleep_start(void){
	volatile unsigned int i;

	REG_ADDR8(0x5a1) &= 0x0f; //MSPI ie disable
	WRITE_REG8(0x80006f,0x81);
	for(i=0; i<0x30; i++);
	REG_ADDR8(0x5a1) |= 0xf0; //MSPI ie enable
}

_attribute_no_inline_ u32		cpu_get_32k_tick ()
{
	u32		t0 = 0, t1 = 0, n;

	n = 0;
	//REG_ADDR8(0x74c) = 0x90;							//system timer auto mode enable
	REG_ADDR8(0x74c) = 0x28;							//system timer manual mode, interrupt disable
	while (1)
	{
		REG_ADDR8(0x74f) = BIT(3);							//start read
		while (REG_ADDR8(0x74f) & BIT(3));
		t0 = t1;
		t1 = REG_ADDR32(0x754);
		if (n)
		{
			if ((u32)(t1 - t0) < 2)
			{
				return t1;
			}
			else if ( (t0^t1) == 1 )	// -1
			{
				return t0;
			}
		}
		n++;
	}
	return t1;
}

__attribute__((section(".ram_code"))) int PM_LowPwrEnter (int deepsleep, int wakeup_src, u32 wakeup_tick)
{
	u16 tick_32k_calib = REG_ADDR16(0x750);  //用于校验32k, 32k 走16个cycle对应几个系统时钟
	u16 tick_32k_halfCalib = tick_32k_calib>>1;


	u8 long_suspend = 0;
	u32 span = (u32)(wakeup_tick - ClockTime()); //计算suspend 时间长度

	if(wakeup_src & WAKEUP_SRC_TIMER)
	{
		if (span > 0xc0000000)  //BIT(31)+BIT(30)   3/4 cylce of 32bit
		{
			return  ReadAnalogReg (0x44) & 0x0f;
		}
		else if (span < EMPTYRUN_TIME_US * TickPerUs) // 0 us base
		{
			u32 t = ClockTime();
			WriteAnalogReg (0x44, 0x0f);			//clear all status

			u8 st;
			do {
				st = ReadAnalogReg (0x44) & 0x0f;
			} while ( ((u32)ClockTime() - t < span) && !st);
			return st;
		}
		else
		{
			if( span > 0x0ff00000 ){  //BIT(28) = 0x10000000   16M:16S; 32M:8S  48M: 5.5S
				long_suspend = 1;
			}
		}
	}
	////////// disable IRQ  and save irq state//////////////////////////////////////////
    unsigned char r = REG_ADDR8(0x643); //irq disable
    REG_ADDR8(0x643) = 0;


	u32 tick_cur = ClockTime();
	u32 tick_32k_cur = cpu_get_32k_tick ();
	u32 tick_wakeup_reset = wakeup_tick - EARLYWAKEUP_TIME_US * TickPerUs;  //计算提请醒来的时间点



	/////////////////// set wakeup source /////////////////////////////////
	///////////////// store gpio wakeup enable setting ////////////////////
	WriteAnalogReg (0x26, wakeup_src);  //设置唤醒源

	WRITE_REG8(0x6e, wakeup_src & BIT(5) ? 0x08 : 0x00);
	WriteAnalogReg (0x44, 0x0f);				//clear wakeup flag

	////////////////////////////// set wakeup tick ////////////////////////////
	u8 rc32k_pd = 0;
	if (wakeup_src & WAKEUP_SRC_TIMER ) {
		rc32k_pd = 0x00;						//32K RC need be enabled
	}
	else {
		WRITE_REG8(0x74c,0x20);
		rc32k_pd = 0x01;
	}

	///////////////////////// change to 32M RC clock before suspend /////////////
	u8 reg66 = READ_REG8(0x066);			//
	WRITE_REG8 (0x066, 0);				//change to 32M rc clock

	WriteAnalogReg(0x2c, (deepsleep ? 0xfe : 0x5e) | rc32k_pd);
	WriteAnalogReg(0x2d, ReadAnalogReg(0x2d) & 0xdf);


	span = (RESET_TIME_US * TickPerUs * 16 + tick_32k_halfCalib)/ tick_32k_calib;

	u32 tick_reset;
	if(long_suspend)
	{
		tick_reset = tick_32k_cur + (u32)(tick_wakeup_reset - tick_cur)/ tick_32k_calib * 16;
	}
	else
	{
		tick_reset = tick_32k_cur + ((u32)(tick_wakeup_reset - tick_cur) * 16 + tick_32k_halfCalib) / tick_32k_calib;
	}


	u8 rst_cycle =  0xff - span;
	WriteAnalogReg (0x20, SWAP_BIT0_BIT6(rst_cycle));				// quick wake-up, short reset time
	REG_ADDR8(0x74c) = 0x2c;
	REG_ADDR32(0x754) = tick_reset;
	REG_ADDR8(0x74f) = BIT(3);									//start write

	while (REG_ADDR8(0x74f) & BIT(3));
	WriteAnalogReg (0x44, 0x0f);								//clear all flag

	if(ReadAnalogReg(0x44)&0x0f){

	}
	else{
		sleep_start();
	}
	if(deepsleep){
		WRITE_REG8 (0x6f, 0x20);  //reboot
	}

	/*suspend recover setting*/
	WriteAnalogReg (0x2c, 0x00);


	if(long_suspend){
		tick_cur += (u32)(cpu_get_32k_tick () - tick_32k_cur) / 16 * tick_32k_calib;
	}
	else{
		tick_cur += (u32)(cpu_get_32k_tick () - tick_32k_cur) * tick_32k_calib / 16;		// current clock
	}


	WRITE_REG8 (0x066, reg66);			//restore system clock
    REG_ADDR32(0x740) = tick_cur;
	REG_ADDR8(0x74c) = 0x00;
	//REG_ADDR8(0x74c) = 0x90;
	REG_ADDR8(0x74c) = 0x92;  //reg_system_tick_mode |= FLD_SYSTEM_TICK_IRQ_EN;
	REG_ADDR8(0x74f) = BIT(0);


	u8 anareg44 = ReadAnalogReg(0x44);


	if (anareg44 & WAKEUP_STATUS_TIMER )	//wakeup from timer only
	{
		while ((u32)(ClockTime() -  wakeup_tick) > BIT(30));
	}


	REG_ADDR8(0x643) = r; //restore the irq

	return anareg44;
}
