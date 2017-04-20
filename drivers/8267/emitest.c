/********************************************************************************************************
 * @file     emitest.c
 *
 * @brief    This file provides set of functions for emi test
 *
 * @author   kaixin.chen@telink-semi.com
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
#include "rf.h"
#include "bsp.h"

#define STATE0		0x1234
#define STATE1		0x5678
#define STATE2		0xabcd
#define STATE3		0xef01

unsigned char  emi_var[5];
unsigned char  emi_tx[16]  __attribute__ ((aligned (4))) = {0xc,0x00,0x00,0x00,0x00,0x20,0xaa,0xbb};
int 		   state0,state1,state2,state3;
unsigned char  depth=1;


void RF_EmiInit(void)
{
	// for registers recover.
	emi_var[0] = ReadAnalogReg(0xa5);
	emi_var[1] = READ_REG8(0x8004e8);
	emi_var[3] = READ_REG8(0x80050f);
	emi_var[4] = READ_REG8(0x80050e);
}

int RF_EmiCarrierRecovery(void)
{
	//reset zb & dma
	WRITE_REG16(0x800060, 0x0480);
	WRITE_REG16(0x800060, 0x0000);

	WriteAnalogReg (0xa5, emi_var[0]);
	WRITE_REG8 (0x8004e8, emi_var[1]);
    if(( g_RFMode == RF_MODE_BLE_2M ) )
	{
    	WRITE_REG8 (0x800402, 0x2b);
	}
	else if(g_RFMode == RF_MODE_BLE_1M )
	{
		WRITE_REG8 (0x800402, 0x2b);
	}
	else if(g_RFMode == RF_MODE_ZIGBEE_250K )
	{
		WRITE_REG8 (0x800402, 0x26);
	}
    WRITE_REG8(0x80050f, emi_var[3]);
    WRITE_REG8(0x80050e, emi_var[4]);
    return 1;

}
#if 0
void phyTest_PRBS9 (unsigned char *p, int n)
{
	//PRBS9: (x >> 1) | (((x<<4) ^ (x<<8)) & 0x100)
	unsigned short x = 0x1ff;
	int i;
	int j;
	for ( i=0; i<n; i++)
	{
		unsigned char d = 0;
		for (j=0; j<8; j++)
		{
			if (x & 1)
			{
				d |= BIT(j);
			}
			x = (x >> 1) | (((x<<4) ^ (x<<8)) & 0x100);
		}
		*p++ = d;
	}
}
#else
//unsigned char prbs9[128];
void phyTest_PRBS9 (unsigned char *p, int n,unsigned char words)
{
	//PRBS9: (x >> 1) | (((x<<4) ^ (x<<8)) & 0x100)
	unsigned short x = 0x1ff;
	int i;
	int j;
	for ( i=0; i<n; i++)
	{
		unsigned char d = 0;
		for (j=0; j<8; j++)
		{
			if (x & 1)
			{
				d |= BIT(j);
			}
			x = (x >> 1) | (((x<<4) ^ (x<<8)) & 0x100);
		}
		if(( i<((words+1)*4))&&( i>= (words*4)))
		{
			*p++ = d;
		}

	}
}
#endif
int pnGen(int state)
{
	int feed = 0;
	feed = (state&0x4000) >> 1;
	state ^= feed;
	state <<= 1;
	state = (state&0xfffe) + ((state&0x8000)>>15);
	return state;
}

void RF_EmiCarrierOnlyTest(RF_PowerTypeDef PowerLevel,signed char RF_Channel)
{
	RF_EmiCarrierRecovery();
	RF_TrxStateSet(RF_MODE_TX,RF_Channel);
	WaitUs(150);//wait pllclock

	RF_PowerLevelSet(PowerLevel);
	WriteAnalogReg(0xa5,0x44);   // for carrier  mode
	WRITE_REG8 (0x8004e8, 0x04); // for  carrier mode
}


void RF_EmiCarrierDataTest(RF_PowerTypeDef PowerLevel,signed char RF_Channel)
{

	int i;

	RF_EmiCarrierRecovery();
	
	RF_PowerLevelSet(PowerLevel);
	RF_TrxStateSet(RF_MODE_TX,RF_Channel);
	WaitUs(150);//wait pllclock

	WRITE_REG8(0x80050e,depth); // this size must small than the beacon_packet dma send length

	state0 = STATE0;
	state1 = STATE1;
	state2 = STATE2;
	state3 = STATE3;
	emi_tx[0] = depth*16-4;
	WRITE_REG8(0x80050f, 0x80);  // must fix to 0x80
	WRITE_REG8(0x800402, 0x21);	//preamble length=1
    RF_TxPkt(emi_tx);
}


void RF_EmiDataUpdate(void)
{
	//write_reg32((emi_tx+depth*16-4),(state0<<16)+state1); // the last value
//	//advance PN generator
	//state0 = pnGen(state0);
	//state1 = pnGen(state1);
	//write_reg32((emi_tx+depth*16-4),0xf0f0f0f0); // the last value
	//phyTest_PRBS9((emi_tx+12),0x04);
	int i;
	for(i=0;i<64;i++)
	{
		phyTest_PRBS9((emi_tx+depth*16-4),64,i);
	}


}


void RF_EmiRxTest(unsigned char *Addr,signed char RF_Channel,signed char BufferSize,unsigned char  PingpongEn)
{
	RF_RxBufferSet(Addr,BufferSize,PingpongEn);
	RF_TrxStateSet(RF_MODE_RX,RF_Channel);
	WaitUs(200);//wait pllclock
	RF_EmiCarrierRecovery();

}


void RF_EmiTxInit(RF_PowerTypeDef PowerLevel,signed char RF_Channel)
{

	RF_PowerLevelSet(PowerLevel);
	RF_TrxStateSet(RF_MODE_TX,RF_Channel);
	WaitUs(200);//wait pllclock
	RF_EmiCarrierRecovery();

}

void RF_EmiSingleTx(unsigned char *Addr)
{
	RF_TxPkt(Addr);
	while(RF_TxFinish()==0);
	RF_TxFinishClearFlag();
}
