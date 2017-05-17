/********************************************************************************************************
 * @file     rf.c
 *
 * @brief    This file provides set of functions for RF module
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
#include "bsp.h"
#include "rf.h"
#include "../../vendor/coor/pa.h"



#define 	REG_RST0			REG_ADDR8(0x60)

//enum{
//	FLD_RST_ZB =				BIT(7),
//};
#define 	REG_RF_ACCESS_CODE_LEN			REG_ADDR8(0x405)
enum {
	FLD_ACCESSZ_CODE_LEN =	BIT_RNG(0,2),
};
#define 	REG_RF_ACCESS_CODE0_L			REG_ADDR32(0x408)
#define 	REG_RF_ACCESS_CODE0_H			REG_ADDR8(0x40c)

#define 	REG_RF_ACCESS_CODE1_L			REG_ADDR32(0x410)
#define 	REG_RF_ACCESS_CODE1_H			REG_ADDR8(0x414)

#define 	REG_RF_ACCESS_CODE2_5_prefix(i)		REG_ADDR8(0x418+i)


#define 	REG_RF_PIPE_RX_CHN_EN			REG_ADDR8(0x407)
enum {
	FLD_RF_RX_ACCESS_CODE_CHN_EN =			BIT_RNG(0,5),
};

#define 	REG_RF_ACCESS_CODE_GTH			REG_ADDR8(0x420)

#define 	REG_RF_RX_MODE				REG_ADDR8(0x428)
enum {
	FLD_RF_RX_ENABLE =					BIT(0),
};
#define 	REG_RF_RX_SATH				REG_ADDR8(0x433)
enum {
	FLD_RF_MGAIN_EN =					BIT(0),
};

#define 	REG_RF_RX_PEL0				REG_ADDR8(0x434)
enum {
	FLD_RF_RX_POW_THD_LOW =					BIT_RNG(4,6),
};
#define 	REG_RF_RX_PCHG				REG_ADDR8(0x43a)
#define 	REG_RF_RX_FGAIN				REG_ADDR8(0x43e)
#define 	REG_RF_RET_RXPGA			REG_ADDR8(0x4cd)

#define 	REG_RF_PN_INI				REG_ADDR8(0x40d)
#define 	REG_RF_TX_CHANNEL			REG_ADDR16(0x4d6)

#define 	REG_RF_RX_MAX_LEN			REG_ADDR8(0x42c)

#define 	REG_RF_RX_GAIN				REG_ADDR8(0x440)
enum {
	FLD_RF_RX_GAIN =					BIT_RNG(0,4),
};
#define 	REG_RF_RX_PEAK				REG_ADDR8(0x446)
enum {
	FLD_RF_RX_PEAK =					BIT_RNG(0,5),
};
#define 	REG_DMA_RF_RX_ADDR			REG_ADDR16(0x508)
#define 	REG_DMA_RF_RX_BUF_SIZE		REG_ADDR8(0x50a)
#define 	REG_DMA_RF_RX_BUF_PIPO		REG_ADDR8(0x50b)

#define 	REG_DMA_RF_TX_ADDR			REG_ADDR16(0x50c)
#define 	REG_DMA_RF_TX_BUF_SIZE		REG_ADDR8(0x50e)
#define 	REG_DMA_RF_TX_BUF_PIPO		REG_ADDR8(0x50f)
#define		REG_DMA_RF_TX_RDY0			REG_ADDR8(0x524)

#define 	REG_DMA_RF_RX_RDY(i)		REG_ADDR8(0x526+i)

enum {
	FLD_DMA_RF_RX =				BIT(2),
	FLD_DMA_RF_TX =				BIT(3),
};

#define 	REG_RF_FSM_CMD				REG_ADDR8(0xf00)
enum {
	FLD_FSM_RF_BRX =	2,
	FLD_FSM_RF_STX =	5,
	FLD_FSM_RF_SRX =	6,
	FLD_FSM_RF_STR =  	7,
	FLD_FSM_RF_SRT =  	8,
	FLD_FSM_RF_CMD_TRIGGER = BIT(7),
};
#define		REG_RF_FUNCTION0_EN			REG_ADDR8(0xf02)

enum {
	FLD_RF_TX_MANUAL_EN =	BIT(4),
	FLD_RF_RX_MANUAL_EN =  	BIT(5),
};
#define		REG_RF_RX_TIMEOUT_US		REG_ADDR16(0xf0a)

#define		REG_RF_FUNCTION1_EN			REG_ADDR8(0xf16)

#define		REG_RF_TX_ACCESS_CODE_CHN		REG_ADDR8(0xf15)
enum {
	FLD_TX_ACCESSZ_CODE_CHN =	BIT_RNG(0,2),
};


#define		REG_RF_CMD_START_TICK		REG_ADDR32(0xf18)
#define		REG_RF_RX_FIRST_TIMEOUT_US	REG_ADDR32(0xf28)
enum {
	FLD_RF_CMD_SCHEDULE_EN =	BIT(2),
};

#define		REG_RF_IRQ_STATUS			REG_ADDR8(0xf20)
enum {
	FLD_RF_IRQ_TX =			BIT(1),
};


#define		REG_RF_FSM_STATUS			REG_ADDR8(0xf24)


#define		GAIN_BASE		aTPGain0[g_RFMode]
#define		GAIN_G			((aTPGain0[g_RFMode] - aTPGain1[g_RFMode])*256/80)


#define 	REG_RF_TP_CAP 				0x93
#define 	REG_RF_FREOFFSET_CAP 		0x81

unsigned  char  aTPGain0[5] = {0x40,0x1d,0x40,0x40,0x1d};
unsigned  char  aTPGain1[5] = {0x39,0x19,0x39,0x39,0x19};
RF_ModeTypeDef   g_RFMode;
unsigned char    g_RFRxPingpongEn;



const unsigned char TblAgc[] = {
	0x31,0x32,0x33,0x30,0x38,0x3c,0x2c,0x18 ,0x1c,0x0c,0x0c,0x00,0x00,0x00,0x00,0x00,
	0x0a,0x0f,0x15,0x1b,0x21,0x27,0x2e,0x32 ,0x38,0x3e
};

const BSP_TblCmdSetTypeDef  TblRFIni[] = {

	0x06, 0x00,  TCMD_UNDER_BOTH | TCMD_WAREG, //power down control.
	0x8f, 0x38,  TCMD_UNDER_BOTH | TCMD_WAREG, //boot rx vco current, temporary fix
	0xa2, 0x2c,  TCMD_UNDER_BOTH | TCMD_WAREG, //pa_ramp_target ****0-5bit

	0xa0, 0x28,  TCMD_UNDER_BOTH | TCMD_WAREG, //dac datapath delay ******change  remington 0x26

	0x04, 0x7c,  TCMD_UNDER_BOTH | TCMD_WAREG,
	0xa7, 0x61,  TCMD_UNDER_BOTH | TCMD_WAREG,
	0x8d, 0x67,  TCMD_UNDER_BOTH | TCMD_WAREG,
	0xac, 0xa7,  TCMD_UNDER_BOTH | TCMD_WAREG,	/////analog reg 0xac,change 0xaa to 0xa7,based on sihui 

	0x81, 0xd0,  TCMD_UNDER_BOTH | TCMD_WAREG,  	//same as application[7:6]xtl16M cor ciruit contrl [4:0]xtl16M_csel_bit
	0x04ca, 0x88,	TCMD_UNDER_BOTH | TCMD_WRITE,	// ********enable DC distance bit 3
	0x04cb, 0x04,	TCMD_UNDER_BOTH | TCMD_WRITE,	// ********eset distance
	0x042d, 0x33,	TCMD_UNDER_BOTH | TCMD_WRITE,	// ********DC alpha=1/8, bit[6:4]
	
	0x043b, 0xfc, TCMD_UNDER_BOTH | TCMD_WRITE,	//rx packet Select DC(freoffset),Select stamp
	
	//AGC_table
	//0x430,0x12, TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x43d,0xb1, TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x438,0xb7, TCMD_UNDER_BOTH | TCMD_WRITE,


};

//RxMaxGain
const BSP_TblCmdSetTypeDef  TblRFIniManualAgc[] = {               // 16M crystal
	0x0433, 0x00,	TCMD_UNDER_BOTH | TCMD_WRITE,	// set mgain disable 01 -> 00
	0x0434, 0x01,	TCMD_UNDER_BOTH | TCMD_WRITE,	// pel0: 21 -> 01
	0x043a, 0x77,	TCMD_UNDER_BOTH | TCMD_WRITE,	// Rx signal power change threshold: 22 -> 77
	0x043e, 0xc9,	TCMD_UNDER_BOTH | TCMD_WRITE,	// set rx peak detect manual: 20 -> c9
	0x04cd, 0x06,	TCMD_UNDER_BOTH | TCMD_WRITE,	// fix rst_pga=0: len = 0 enable
};


const BSP_TblCmdSetTypeDef  TblRFIni16M[] = {               // 16M crystal
	//0x04eb, 0x60,	TCMD_UNDER_BOTH | TCMD_WRITE,   // auto
	//0x99, 	0x31,  TCMD_UNDER_BOTH | TCMD_WAREG,	// gauss filter sel: 16M
	//0x82,	0x14,  TCMD_UNDER_BOTH | TCMD_WAREG,	// gauss filter sel: 16M
	
};

const BSP_TblCmdSetTypeDef  TblRFIni12M[] = {               // 12M crystal
	//////0x04eb, 0xe0,  TCMD_UNDER_BOTH | TCMD_WRITE,	// RX freq regisgter: 4d4
	//////0x99, 	0xb1,  TCMD_UNDER_BOTH | TCMD_WAREG,	// gauss filter sel: 16M
	////////0x82,	0x00,  TCMD_UNDER_BOTH | TCMD_WAREG,	//enable rxadc clock
	//0x9e, 	0x82,  TCMD_UNDER_BOTH | TCMD_WAREG, 	//reg_dc_mod (500K)
};

// rf mode
////////////////////////////////////////////////////////////////////////////////////////////////

const BSP_TblCmdSetTypeDef  TblRFZigbee250k[] = {
	  0x9e, 0xad,  TCMD_UNDER_BOTH | TCMD_WAREG,   //reg_dc_mod (500K); ble: 250k
	  0xa3, 0x10,  TCMD_UNDER_BOTH | TCMD_WAREG,   //pa_ramp_en = 1, pa ramp table max
	  0xaa, 0x2a,  TCMD_UNDER_BOTH | TCMD_WAREG,  //filter iq_swap, 2M bandwidth
	  0x93, 0x50,  TCMD_UNDER_BOTH | TCMD_WAREG,

	  0x0400, 0x03, TCMD_UNDER_BOTH | TCMD_WRITE, // 250K mode
	  0x0401, 0x40, TCMD_UNDER_BOTH | TCMD_WRITE, // pn enable
	  0x0402, 0x26, TCMD_UNDER_BOTH | TCMD_WRITE, // 8-byte pre-amble
	  0x0404, 0xc0, TCMD_UNDER_BOTH | TCMD_WRITE, // head_mode/crc_mode: normal c0
	  0x0405, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // access code length 4
	  0x0408, 0xc9, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte3
	  0x0409, 0x8a, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte2
	  0x040a, 0x11, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte1
	  0x040b, 0xf8, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte0


	  0x0420, 0x90, TCMD_UNDER_BOTH | TCMD_WRITE, // sync threshold: 1e (4); 26 (5)
	  0x0421, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // no avg
	  0x0422, 0x1a, TCMD_UNDER_BOTH | TCMD_WRITE, // threshold
	  0x0424, 0x52, TCMD_UNDER_BOTH | TCMD_WRITE, // number for sync: bit[6:4]
	  0x0428, 0xe0, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x042b, 0xf5, TCMD_UNDER_BOTH | TCMD_WRITE, // 8 symbol
	  0x042c, 0x88, TCMD_UNDER_BOTH | TCMD_WRITE, // maxiumum length 48-byte
	
	 // 0x043b, 0xac, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x0464, 0x07, TCMD_UNDER_BOTH | TCMD_WRITE,//new sync
	  0x04cd, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // bit3: crc2_en; normal 1e

	  //AGC_table
	//  0x043a, 0x33, TCMD_UNDER_BOTH | TCMD_WRITE,
	  //0x04c0, 0x81, TCMD_UNDER_BOTH | TCMD_WRITE,
	  
	  0x04eb, 0xa0, TCMD_UNDER_BOTH | TCMD_WRITE,//[6]lowswap for rx integer/frac auto calculation(for zigbee +4M problem)


};

const BSP_TblCmdSetTypeDef  TblRFBle2m[] = {

	0x9e, 0xad,  TCMD_UNDER_BOTH | TCMD_WAREG, 	//*****reg_dc_mod (500KHz*128/126)****remington  	reg_dc_mod (500K)	0x82
	0xa3, 0xd0,  TCMD_UNDER_BOTH | TCMD_WAREG,//********[7:6] disable gauflt [5] LUT 2M or 1M
	0x93, 0x50,  TCMD_UNDER_BOTH | TCMD_WAREG,//*****dac gain settting
	0xaa, 0x2e,  TCMD_UNDER_BOTH | TCMD_WAREG,//*******filter iq_swap, adjust the bandwidth*****remington 0x2e

	0x0400, 0x0f, TCMD_UNDER_BOTH | TCMD_WRITE, // new 2M mode
	0x0401, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0402, 0x2b, TCMD_UNDER_BOTH | TCMD_WRITE, // 8-byte pre-amble
	0x0404, 0xc5, TCMD_UNDER_BOTH | TCMD_WRITE, // head_mode/crc_mode: normal c5

	0x0405, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // access code length 4
	0x0408, 0xf8, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte3
	0x0409, 0x11, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte2
	0x040a, 0x8a, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte1
	0x040b, 0xc9, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte0

	0x0420, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // sync threshold: 1e (4); 26 (5)
	0x0421, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // no avg
	0x0422, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // threshold
	0x0424, 0x12, TCMD_UNDER_BOTH | TCMD_WRITE, // number fo sync: bit[6:4]
	0x0428, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x042b, 0xf1, TCMD_UNDER_BOTH | TCMD_WRITE, // access code: 1
	0x042c, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,

	0x0464, 0x07, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04cd, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // enable packet lenght = 0
	0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE,

	//0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04eb, 0xe0, TCMD_UNDER_BOTH | TCMD_WRITE,////[6]lowswap for rx integer/frac auto calculation(for zigbee +4M problem)
};


// ble 1m mode
const BSP_TblCmdSetTypeDef  TblRFBle1m[] = {

	0x9e, 0x56,  TCMD_UNDER_BOTH | TCMD_WAREG, 	//*****reg_dc_mod (500KHz*128/126)****remington  	reg_dc_mod (500K)	0x82
	0xa3, 0xf0,  TCMD_UNDER_BOTH | TCMD_WAREG,//********[7:6] disable gauflt [5] LUT 2M or 1M
	0x93, 0x28,  TCMD_UNDER_BOTH | TCMD_WAREG,//*****dac gain settting
	0xaa, 0x26,  TCMD_UNDER_BOTH | TCMD_WAREG,//*******filter iq_swap, adjust the bandwidth*****remington 0x2e

	0x0400, 0x0f, TCMD_UNDER_BOTH | TCMD_WRITE, // new 2M mode
	0x0401, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0402, 0x2b, TCMD_UNDER_BOTH | TCMD_WRITE, // 8-byte pre-amble
	0x0404, 0xd5, TCMD_UNDER_BOTH | TCMD_WRITE, // head_mode/crc_mode: normal c5
	0x0405, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0408, 0xc9, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0409, 0x8a, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x040a, 0x11, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x040b, 0xf8, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0420, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // sync threshold: 1e (4); 26 (5)
	0x0421, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // no avg
	0x0422, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // threshold
	0x0424, 0x12, TCMD_UNDER_BOTH | TCMD_WRITE, // number fo sync: bit[6:4]
	0x0428, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x042b, 0xf1, TCMD_UNDER_BOTH | TCMD_WRITE, // access code: 1
	0x042c, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,

	0x0464, 0x07, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04cd, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // enable packet lenght = 0
	0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE,

	//0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04eb, 0xe0, TCMD_UNDER_BOTH | TCMD_WRITE,//[6]lowswap for rx integer/frac auto calculation(for zigbee +4M problem)
};


const BSP_TblCmdSetTypeDef  TblRFStandardBle1m[] = {

	0x9e, 0x56,  TCMD_UNDER_BOTH | TCMD_WAREG, 	//*****reg_dc_mod (500KHz*128/126)****remington  	reg_dc_mod (500K)	0x82
	0xa3, 0xf0,  TCMD_UNDER_BOTH | TCMD_WAREG,//********[7:6] disable gauflt [5] LUT 2M or 1M
	0x93, 0x28,  TCMD_UNDER_BOTH | TCMD_WAREG,//*****dac gain settting
	0xaa, 0x26,  TCMD_UNDER_BOTH | TCMD_WAREG,//*******filter iq_swap, adjust the bandwidth*****remington 0x2e

	0x0400, 0x0f, TCMD_UNDER_BOTH | TCMD_WRITE, // new 2M mode
	0x0401, 0x08, TCMD_UNDER_BOTH | TCMD_WRITE, // 1M/2M PN enable
	0x0402, 0x24, TCMD_UNDER_BOTH | TCMD_WRITE, // 8-byte pre-amble
	0x0404, 0xf5, TCMD_UNDER_BOTH | TCMD_WRITE, // ble_wt
	0x0405, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0408, 0xc9, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0409, 0x8a, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x040a, 0x11, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x040b, 0xf8, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x0420, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // sync threshold: 1e (4); 26 (5)
	0x0421, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // no avg
	0x0422, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // threshold
	0x0424, 0x12, TCMD_UNDER_BOTH | TCMD_WRITE, // number fo sync: bit[6:4]
	0x0428, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x042b, 0xf1, TCMD_UNDER_BOTH | TCMD_WRITE, // access code: 1
	0x042c, 0x80, TCMD_UNDER_BOTH | TCMD_WRITE,

	0x0464, 0x07, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04cd, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // enable packet lenght = 0
	0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE,

	//0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	//0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04eb, 0xe0, TCMD_UNDER_BOTH | TCMD_WRITE,//[6]lowswap for rx integer/frac auto calculation(for zigbee +4M problem)
};

const BSP_TblCmdSetTypeDef  TblRFPrivate2M[] = {
	  0x9e, 0xad,  TCMD_UNDER_BOTH | TCMD_WAREG,   //reg_dc_mod (500K); ble: 250k
	  0xa3, 0x10,  TCMD_UNDER_BOTH | TCMD_WAREG,   //pa_ramp_en = 1, pa ramp table max
	  0xaa, 0x2e,  TCMD_UNDER_BOTH | TCMD_WAREG,  //filter iq_swap, 2M bandwidth
	  0x93, 0x50,  TCMD_UNDER_BOTH | TCMD_WAREG,

	  0x0400, 0x0b, TCMD_UNDER_BOTH | TCMD_WRITE, // 250K mode
	  0x0401, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // pn enable
	  0x0402, 0x26, TCMD_UNDER_BOTH | TCMD_WRITE, // 8-byte pre-amble
	  0x0404, 0xc0, TCMD_UNDER_BOTH | TCMD_WRITE, // head_mode/crc_mode: normal c0
	  0x0405, 0x04, TCMD_UNDER_BOTH | TCMD_WRITE, // access code length 4
	  0x0408, 0xc9, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte3
	  0x0409, 0x8a, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte2
	  0x040a, 0x11, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte1
	  0x040b, 0xf8, TCMD_UNDER_BOTH | TCMD_WRITE, // access code byte0

	  0x0420, 0x90, TCMD_UNDER_BOTH | TCMD_WRITE, // sync threshold: 1e (4); 26 (5)
	  0x0421, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE, // no avg
	  0x0422, 0x1a, TCMD_UNDER_BOTH | TCMD_WRITE, // threshold
	  0x0424, 0x52, TCMD_UNDER_BOTH | TCMD_WRITE, // number for sync: bit[6:4]
	  0x0428, 0xe4, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x042b, 0xe3, TCMD_UNDER_BOTH | TCMD_WRITE, // access code: 1
	  0x042c, 0x88, TCMD_UNDER_BOTH | TCMD_WRITE, // maxiumum length 48-byte

	  0x0464, 0x06, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x04cd, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // bit3: crc2_en; normal 1e

	  //AGC_table
	 // 0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	 // 0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,

	  0x04eb, 0xe0, TCMD_UNDER_BOTH | TCMD_WRITE,//[6]lowswap for rx integer/frac auto calculation(for zigbee +4M problem)


};

const char TblRFPower[] = {
  //a2    04    a7    8d
	0x25, 0x7c, 0x67, 0x67,		// 7 dBm
	0x0a, 0x7c, 0x67, 0x67,		// 5 dBm
	0x06, 0x74, 0x43, 0x61,		// -0.6
	0x06, 0x64, 0xc2, 0x61,		// -4.3
	0x06, 0x64, 0xc1, 0x61,		// -9.5
	0x05, 0x7c, 0x67, 0x67,		// -13.6
	0x03, 0x7c, 0x67, 0x67,		// -18.8
	0x02, 0x7c, 0x67, 0x67,		// -23.3
	0x01, 0x7c, 0x67, 0x67,		// -27.5
	0x00, 0x7c, 0x67, 0x67,		// -30
	0x00, 0x64, 0x43, 0x61,		// -37
	0x00, 0x64, 0xcb, 0x61,		// -max	power down PA & PAD
};

/**
*	@brief		This function should be invoked first,before the “RF_TrxStateSet”
*				is invoked
*
*	@param[in]	OscSel   enum variable of external crystal RF_OSC_16M/RF_OSC_12M
*	@param[in]	RF_Mode      enum variable of RF mode
*	@return		1 :set success;
*               0 :parameters set error
*/
int	 RF_Init( RF_OscSelTypeDef  OscSel,RF_ModeTypeDef RF_Mode)
{
	unsigned  char   i;
	int temflag;
	LoadTblCmdSet (TblRFIni, sizeof (TblRFIni)/sizeof (BSP_TblCmdSetTypeDef));
	if (OscSel == RF_OSC_12M) {
		LoadTblCmdSet (TblRFIni12M, sizeof (TblRFIni12M)/sizeof (BSP_TblCmdSetTypeDef));
	}
	else {
		LoadTblCmdSet (TblRFIni16M, sizeof (TblRFIni16M)/sizeof (BSP_TblCmdSetTypeDef));
	}
	for (i=0; i<26; i++)
		WRITE_REG8 (0x800480+i, TblAgc[i]);	//set AGC table

	//LoadTblCmdSet (TblRFIniManualAgc, sizeof (TblRFIniManualAgc)/sizeof (BSP_TblCmdSet));

	temflag = RF_ModeSet(RF_Mode);
	RF_SetGainManualMax();
	g_RFMode = RF_Mode;
	return  temflag;
}

/**
*	@brief	  	This function serves to switch to maxgain mode with better
*				Rx performance,	and will only take effect if it’s invoked after
*				the “RF_Init”.The “RF_Init” sets the mode as AGC mode by default.
*
*	@param[in]	length  	Optional range: 3~5
*							Note: The effect for 3-byte access_code is not good.
*
*	@return	 	none
*/
void RF_SetGainManualMax (void)
{
	REG_RF_RX_SATH = 0x00;
	REG_RF_RX_PEL0 = 0x01;
	REG_RF_RX_PCHG =  0x77;//	Rx signal power change threshold
	REG_RF_RX_FGAIN = 0xc9;//  set rx peak detect manual
	if ((RF_MODE_BLE_1M ==g_RFMode)||(RF_MODE_BLE_2M ==g_RFMode)||(RF_MODE_STANDARD_BLE_1M ==g_RFMode)) {
		REG_RF_RET_RXPGA	= 0x06;//  [1:0]fix rst_pga=0 [2]len_0_en(ble)
	}
	else {
		REG_RF_RET_RXPGA = 0x02;//  [1:0]fix rst_pga=0
	}
	WRITE_REG8 (0x800430, 0x17);
	WRITE_REG8 (0x80043d, 0xfd);
	WRITE_REG8 (0x800438, 0x37);

}

/**
*	@brief	  	This function serves to switch to AGC mode from maxgain mode,
*				and will only take effect if it’s invoked after the “RF_Init”.
*
*	@param[in]	none
*
*
*	@return	 	none
*/
void RF_SetAgc (void)
{
	REG_RF_RX_SATH = 0x01;
	REG_RF_RX_PEL0 = 0x21;//  [6:4]pel0
	REG_RF_RX_PCHG =  0x22;//	Rx signal power change threshold
	REG_RF_RX_FGAIN = 0x20;
	if ((RF_MODE_BLE_1M == g_RFMode)||(RF_MODE_BLE_2M == g_RFMode)||(RF_MODE_STANDARD_BLE_1M == g_RFMode)) {
		REG_RF_RET_RXPGA = 0x04;  //[1:0]fix rst_pga=0 [2]len_0_en(ble)
	}
	else {
		REG_RF_RET_RXPGA = 0x00;//[1:0]fix rst_pga=0
	}
	WRITE_REG8 (0x800430, 0x12);
	WRITE_REG8 (0x80043d, 0xb1);
	WRITE_REG8 (0x800438, 0xb7);
}

/**
*	@brief		This function serves to switch RF mode. After this function is
*				invoked, the “RF_TrxStateSet” and “RF_BaseBandReset” should be
*				invoked, as shown below:
*				RF_ModeSet(RF_MODE_BLE_1M);
*		   		RF_TrxStateSet(RF_MODE_TX,20);
*		   		WaitUs(150);//wait pllclock
*		 		RF_BaseBandReset();
*
*	@param[in]	RF_Mode   Set working mode for RF module
*	@return	 	0 :set success;
*              -1 :parameters set error
*/
int RF_ModeSet(RF_ModeTypeDef RF_Mode)
{
	g_RFMode = RF_Mode;
	if (RF_Mode == RF_MODE_BLE_2M) {
		LoadTblCmdSet (TblRFBle2m, sizeof (TblRFBle2m)/sizeof (BSP_TblCmdSetTypeDef));
	}
	else if (RF_Mode == RF_MODE_BLE_1M) {
		LoadTblCmdSet (TblRFBle1m, sizeof (TblRFBle1m)/sizeof (BSP_TblCmdSetTypeDef));
	}
	else if (RF_Mode == RF_MODE_ZIGBEE_250K) {
	   LoadTblCmdSet (TblRFZigbee250k, sizeof (TblRFZigbee250k)/sizeof (BSP_TblCmdSetTypeDef));

	}
	else if (RF_Mode == RF_MODE_PRIVATE_2M) {
	   LoadTblCmdSet (TblRFPrivate2M, sizeof (TblRFPrivate2M)/sizeof (BSP_TblCmdSetTypeDef));

	}
	else if (RF_Mode == RF_MODE_STANDARD_BLE_1M) {
		LoadTblCmdSet (TblRFStandardBle1m, sizeof (TblRFStandardBle1m)/sizeof (BSP_TblCmdSetTypeDef));
	}
	else {
		return  -1;
	}
	return 0;
}

/**
*	@brief	 This function is used during RF mode switch to reset Baseband. 
*   @param   none
*	@return	 none
*/
void RF_BaseBandReset (void)
{
	SET_BIT_FLD(REG_RST0, FLD_RST_ZB);
	CLR_BIT_FLD(REG_RST0, FLD_RST_ZB);
}

static RF_StatusTypeDef RF_TRxState = 0xff;
/**
*	@brief	  	This function is to set Rf channel and state(rx/tx/auto),After
*				this function is invoked and state is rx or tx,a 100us~150us
*				delay is needed to lock PLL. Then it’s ready to carry out packet
*				Rx/Tx operations.
*
*	@param[in]	RF_Status - set current status for RF module(tx/rx/auto),
*						  Note: When the “RF_Status” is set as auto mode, the functions
*						  “RF_StartStx”, “RF_StartStxToRx” and “RF_StartSrxToTx”
*						  can be used.
*	@param[in]	RF_Status - Unit is MHz.
*						  The frequency is set as (2400+ channel) MHz.(0~100)
*						  Note: It should be set as the same value for Rx and Tx
*						  terminal.
*	@return	 	0 :set success; 
*              -1 :parameters set error
*/
int  RF_TrxStateSet(RF_StatusTypeDef  RF_Status,signed char RF_Channel)
{
    int  err = 0;
    if(RF_Status != RF_TRxState)
    	RF_SetTxRxOff();
	if (RF_Status == RF_MODE_TX) {
#if PA_MODE
		Pa_Mode_Switch(PA_TX_MODE);
#endif
		RF_TRxState = RF_MODE_TX;
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL = 2400 + RF_Channel;
		SET_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN);
		CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	}
	else if (RF_Status == RF_MODE_RX) {

#if PA_MODE
		Pa_Mode_Switch(PA_RX_MODE);
#endif
		RF_TRxState = RF_MODE_RX;
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL =  2400 + RF_Channel;
		SET_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_RX_MANUAL_EN);
		SET_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	}
	else if (RF_Status == RF_MODE_AUTO) {
		RF_TRxState = RF_MODE_AUTO;
		REG_RF_FUNCTION1_EN = 0x29;
		CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL = 2400 + RF_Channel;
	}
	else {
	    err = -1;
	}
	WriteAnalogReg(REG_RF_TP_CAP, GAIN_BASE - ((RF_Channel * GAIN_G +128) >> 8));   	// set tp_gain
	//2482--41 2484--42 ....2500--50
	//2400--61 2398--60 ....2380--51
	if (g_RFMode == RF_MODE_STANDARD_BLE_1M) {
		RF_Channel = RF_Channel >> 1;
		if (RF_Channel < 1)
			RF_Channel = 61+RF_Channel;
		else if (RF_Channel < 2)
			RF_Channel = 37;
		else if (RF_Channel < 13)
			RF_Channel -= 2;
		else if (RF_Channel == 13)
			RF_Channel = 38;
		else if (RF_Channel < 40)
			RF_Channel -= 3;
		else if(RF_Channel == 40)
			RF_Channel = 39;
		else if(RF_Channel < 51)
			RF_Channel = RF_Channel;
		REG_RF_PN_INI = RF_Channel;
	}
	return  err;

}

RF_StatusTypeDef RF_TrxStateGet(void)
{
	return RF_TRxState;
}

/**
*	@brief	  	This function is to send packet 
*
*	@param[in]	RF_TxAddr   Tx packet address in RAM. Should be 4-byte aligned.
*					  		 The format for Tx packet in RAM is shown as below:
*					 		(“length” indicates data length, “data” indicates data
*					 		 to be transmitted.)
*	BLE (1M, 2M) mode					Zigbee (250K) mode, Private (2M) mode
*	addr			length+2				addr				Length+1
*	addr+1				0					addr+1					0
*	addr+2				0					addr+2					0
*	addr+3				0					addr+3					0
*	addr+4				0					addr+4				Length+2
*	addr+5			length					addr+5				data(0)
*	addr+6			data(0)					addr+6				data(1)
*	addr+7			data(1)					addr+7				data(2)
*	…	…									…	…
*	addr+5+(length)	data(length-1)			addr+4+(length)		data(length-1)
*					
*	@return	 	none
*/
void RF_TxPkt (unsigned char* RF_TxAddr)
{
	REG_DMA_RF_TX_ADDR 	= (unsigned short)(RF_TxAddr);
	REG_DMA_RF_TX_RDY0 |= FLD_DMA_RF_TX;
}

/**
*	@brief	  	This function serves to start STX mode of auto_mode.
*				This mode will end after a packet is transmitted.
*
*	@param[in]	RF_TxAddr   Tx packet address in RAM. Should be 4-byte aligned.
*	@param[in]	RF_StartTick  Tick value of system timer. It determines when to
*						  	  start STX mode and send packet.
*	@return	 	none
*/
void RF_StartStx  (unsigned char* RF_TxAddr,unsigned int RF_StartTick)
{
#if PA_MODE
		Pa_Mode_Switch(PA_TX_MODE);
#endif

	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);// Enable cmd_schedule mode
	REG_RF_CMD_START_TICK = RF_StartTick;// Setting schedule trigger time
	REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
	SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_STX);// single TX
}

/**
*	@brief	  	This function serves to start Srx mode of auto_mode. In this mode,
*				RF module stays in Rx status until a packet is received or it fails to receive packet when timeout expires. 
*				Timeout duration is set by the parameter "RF_RxTimeoutUs". 
*				The address to store received data is set by the function “RF_RxBufferSet”.
*
*	@param[in]	tick   		  	 Tick value of system timer. It determines when to start Srx mode.
*	@param[in]	timeout_us   	 Unit is us. It indicates timeout duration in Rx status.Max value: 0xffffff (16777215)  
*
*	@return	 	none
*/
void RF_StartSrx(unsigned int RF_StartTick,unsigned int RF_RxTimeoutUs)
{
#if PA_MODE
		Pa_Mode_Switch(PA_RX_MODE);
#endif
	REG_RF_RX_FIRST_TIMEOUT_US = RF_RxTimeoutUs-1;// first timeout
	REG_RF_CMD_START_TICK = RF_StartTick;		// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
	SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_SRX);// single rx
}

/**
*	@brief	  	This function serves to start StxToRx mode of auto_mode.
*				In this mode, a packet is sent first,RF module waits for 10us,
*				stays in Rx status until data is received or timeout expires,
*				then exits this mode.Timeout duration is set by the parameter
*				“RF_RxTimeoutUs”.The address to store received data is set by the
*				function “RF_RxBufferSet”.
*
*	@param[in]	RF_TxAddr  Tx packet address in RAM. Should be 4-byte aligned.
*	@param[in]	RF_StartTick   	Tick value of system timer. It determines when
*								to start StxToRx mode and send packet.
*	@param[in]	RF_RxTimeoutUs  Unit is us. It indicates timeout duration in
*							 	Rx status.Max value: 0xfff (4095)
*
*	@return	 	none
*/
void RF_StartStxToRx  ( unsigned char* RF_TxAddr ,unsigned int RF_StartTick,unsigned short RF_RxTimeoutUs)
{
#if PA_MODE
		Pa_Mode_Switch(PA_TX_MODE);
#endif
	REG_RF_RX_TIMEOUT_US = RF_RxTimeoutUs-1;
	REG_RF_CMD_START_TICK = RF_StartTick;// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
    REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
    SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_STR);// single tx2rx
}


/**
*	@brief	  	This function serves to start SrxToTx mode of auto_mode.
*				In this mode,RF module stays in Rx status until a packet is
*				received or it fails to receive packetwhen timeout expires.
*				If a packet is received within the timeout duration, RF module
*				will wait for 10us,send a packet, and then exit this mode.
*				If it fails to receive packet when timeout expires, RF module
*				will directly exit this mode.Timeout duration is set by the
*				parameter "RF_RxTimeoutUs".	The address to store received data is set
*				by the function “RF_RxBufferSet”.
*
*	@param[in]	RF_TxAddr 	Tx packet address in RAM. Should be 4-byte aligned.
*	@param[in]	RF_StartTick   Tick value of system timer. It determines when to
*								start SrxToTx mode.
*	@param[in]	RF_RxTimeoutUs  Unit is us. It indicates timeout duration in Rx status.
*								Max value: 0xffffff (16777215)
*
*	@return	 	none
*/
void RF_StartSrxToTx  (unsigned char* RF_TxAddr  ,unsigned int RF_StartTick,unsigned int RF_RxTimeoutUs)
{
#if PA_MODE
		Pa_Mode_Switch(PA_RX_MODE);
#endif
	REG_RF_RX_FIRST_TIMEOUT_US = RF_RxTimeoutUs-1;// first timeout
	REG_RF_CMD_START_TICK = RF_StartTick;		// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
    REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
    SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_SRT);	// single rx2tx
}

/**
*	@brief	  	This function is to set rx buffer
*
*	@param[out]	RF_RxAddr  	Pointer for Rx buffer in RAM(Generally it’s starting
*							address of an array.Should be 4-byte aligned)
*	@param[in]	Size   		Rx buffer size (It’s an integral multiple of 16)
*	@param[in]	PingpongEn 	Enable/Disable Ping-Pong buffer 1：Enable 0：Disable
*							Note:
*							When “PingpongEn” is set as 0, received RF data will
*							be stored in RAM pointed by “ RF_RxAddr”.
*							When “PingpongEn” is set as 1, received RF data will
*							be stored in buffer0 and buffer1 successively.
*							The RAM size reserved for received RF data should be
*							double of “Size”.
*
*	@return	 	none
*/
void  RF_RxBufferSet(unsigned char *  RF_RxAddr, int Size, unsigned char  PingpongEn)
{
    unsigned char mode;
	mode = PingpongEn ? 0x03 : 0x01;
	REG_DMA_RF_RX_ADDR = (unsigned short)(RF_RxAddr);
	REG_DMA_RF_RX_BUF_SIZE = Size>>4;
	REG_DMA_RF_RX_BUF_PIPO = mode;
	g_RFRxPingpongEn = PingpongEn;
}

/**
*	@brief	  	This function is to get rx buffer result
*
*	@return	 	0:There are received RF data in buffer 0;
*				1:There are received RF data in buffer 1; 
*				2:No RF data is received.
*/
unsigned char RF_RxBufferRequest(void)
{
    int i;
	int loop;
	unsigned char * ptr;
	loop = g_RFRxPingpongEn ? 2 : 1;
	ptr = NULL;
	for (i=0;i<loop;i++) {
		if (REG_DMA_RF_RX_RDY(i) & FLD_DMA_RF_RX) {// is not empty
			 break;
		}
	}
	return i;   // 0  &  1  

}

/**
*	@brief	  	This function serves to clear the flag bit for Rx buffer.
*				After RF data are received, flag bits will be generated for
*				buffer 0 and buffer 1.By reading corresponding flag bit, it can
*				check whether RF Rx event occurs.After the received data
*				are processed, it’s needed to clear flag bits for buffer 0
*				and buffer 1,so as to avoid regarding the old data as new by mistake.

*	@param[in]	RF_RxBufferIdx   0:buffer0  1:buffer1
*
*	@return	 	none
*/
void  RF_RxBufferClearFlag(unsigned char  RF_RxBufferIdx)
{
	SET_BIT_FLD(REG_DMA_RF_RX_RDY(RF_RxBufferIdx) , FLD_DMA_RF_RX);
}

/**
*	@brief	  	Check if all packet data are sent. 
*
*	@return	 	0x00: Packet Tx is not finished yet;
*				0x02: Packet Tx is finished.
*/
unsigned char  RF_TxFinish(void)
{
	return  (REG_RF_IRQ_STATUS & FLD_RF_IRQ_TX);
}

/**
*	@brief	  	This function serves to clear the Tx finish flag bit.
*				After all packet data are sent, corresponding Tx finish flag bit
*				will be set as 1.By reading this flag bit, it can check whether
*				packet transmission is finished. After the check, it’s needed to
*				manually clear this flag bit so as to avoid misjudgment.
*   @param      none
*	@return	 	none
*/
void RF_TxFinishClearFlag (void)
{
	SET_BIT_FLD(REG_RF_IRQ_STATUS , FLD_RF_IRQ_TX);
}

/**
*	@brief	  	Set index number of access_code channel for RF Tx terminal. 
*
*	@param[in]	RF_TxPipeIdx  	Optional range: 0~5
*
*	@return	 	none
*/
void RF_TxAccessCodeSelect (unsigned char RF_TxPipeIdx)
{
	CLR_BIT_FLD(REG_RF_TX_ACCESS_CODE_CHN ,FLD_TX_ACCESSZ_CODE_CHN);
	REG_RF_TX_ACCESS_CODE_CHN |= RF_TxPipeIdx;//Tx_Channel_man[2:0]

}

/**
*	@brief	  	this function is to Set byte length for access_code.
*
*	@param[in]	RF_AccessCodeLength  	Optional range: 3~5
*										Note: The effect for 3-byte access_code is not good.
*
*	@return	 	none
*/
void RF_AccessCodeLengthSetting (unsigned char RF_AccessCodeLength)
{
	CLR_BIT_FLD(REG_RF_ACCESS_CODE_LEN ,FLD_ACCESSZ_CODE_LEN);
	REG_RF_ACCESS_CODE_LEN |= RF_AccessCodeLength;//access_byte_num[2:0]
	if (5 == RF_AccessCodeLength) {
		REG_RF_ACCESS_CODE_GTH = 0x28;//access_bit_threshold

	}
	else if (4 == RF_AccessCodeLength) {
		REG_RF_ACCESS_CODE_GTH = 0x20;//access_bit_threshold

	}
	else if (3 == RF_AccessCodeLength) {
		REG_RF_ACCESS_CODE_GTH = 0x18;//access_bit_threshold
	}
}

/**
*	@brief	  	this function is to set access code of 0/1 channel 
*
*	@param[in]	RF_PipeIdx  	Set index number for access_code channel (only 0 or 1)
*	@param[in]	RF_AccessCode  	Set 3~5 bytes access_code value (should be consistent
*								with “length”)
*								Note: Access_code configuration is determined by
*								algorithm, random configuration may influence
*								RF Rx/Tx performance.
*
*	@return	 	none
*/
void RF_AccessCodeSetting01 (unsigned char RF_PipeIdx,unsigned long long  RF_AccessCode)
{
	unsigned char length=0;
	length = REG_RF_ACCESS_CODE_LEN;
	if (5 == length) {
		if (0 == RF_PipeIdx) {
			REG_RF_ACCESS_CODE0_L = (unsigned int)RF_AccessCode;
			REG_RF_ACCESS_CODE0_H = (unsigned char)(RF_AccessCode>>32);

		}
		else if (1 == RF_PipeIdx) {
			REG_RF_ACCESS_CODE1_L = (unsigned int)RF_AccessCode;
			REG_RF_ACCESS_CODE1_H = (unsigned char)(RF_AccessCode>>32);
		}
	}
	else if (4 == length) {
		if (0 == RF_PipeIdx) {
			REG_RF_ACCESS_CODE0_L = (unsigned int)RF_AccessCode;
		}
		else if (1 == RF_PipeIdx) {
			REG_RF_ACCESS_CODE1_L = (unsigned int)RF_AccessCode;
		}
	}
	else if(3 == length) {
		if (0 == RF_PipeIdx) {
			REG_RF_ACCESS_CODE0_L &= 0xff000000;
			REG_RF_ACCESS_CODE0_L |= (unsigned int)RF_AccessCode;
		}
		else if (1 == RF_PipeIdx){
			REG_RF_ACCESS_CODE1_L &= 0xff000000;
			REG_RF_ACCESS_CODE1_L |= (unsigned int)RF_AccessCode;
		}

	}
}


/**
*	@brief	  	this function is to set access code of 2/3/4/5 channel 
*
*	@param[in]	RF_PipeIdx  	Set index number for access_code channel (only 2~5).
*	@param[in]	Prefix  Set 1-byte prefix of access_code.
*						Note:When this function is invoked, the
*						“RF_AccessCodeSetting01” should also be	invoked to set
*						access_code1 for channel 1.	Following is an example to
*						set access_code2 for channel 2.
*						//set access_code length as 5 bytes
*						RF_AccessCodeLengthSetting (5);
*						RF_AccessCodeSetting01 (1, 0x1122334455);
*						RF_AccessCodeSetting2345 (2, 0x66);
*						By the configuration above, the access_code value for
*						channel 2 (i.e. access_code2)is set as “0x1122334466”
*						(Former 4 bytes are higher 4 bytes of access_code1,
*						while the final byte is obtained from the
*						“AccessCodeSetting2345” function).	The “RF_AccessCodeSetting2345”
*						can only set the prefix for access_code(The prefix is the
*						lowest byte of access_code).For channel 2~5, the whole
*						access_code must be set via access_code1higher 4 bytes
*						of channel 1 and pefix.
*
*	@return	 	none
*/
void RF_AccessCodeSetting2345 (unsigned char  RF_PipeIdx,unsigned char  Prefix)
{
	REG_RF_ACCESS_CODE2_5_prefix(RF_PipeIdx-2) = Prefix;//access_code 2~5
}

/**
*	@brief		this function is to enable/disable each access_code channel for
*				RF Rx terminal.
*
*	@param[in]	RF_RxPipeSel  	Bit0~bit5 correspond to channel 0~5, respectively.
*								0：Disable 1：Enable
*								If “enable” is set as 0x3f (i.e. 00111111),
*								all access_code channels (0~5) are enabled.
*
*	@return	 	none
*/
void RF_RxAccessCodeEnable (unsigned char RF_RxPipeSel)
{
	CLR_BIT_FLD(REG_RF_PIPE_RX_CHN_EN ,FLD_RF_RX_ACCESS_CODE_CHN_EN);
	REG_RF_PIPE_RX_CHN_EN |= RF_RxPipeSel;//rx_access_code_chn_en
}

/**
*	@brief	  	This function is to select Tx power level. Parameter type is enum. 
*
*	@param[in]	RF_TxPowerLevel   select Tx power level
*
*	@return	 	none
*/
void RF_PowerLevelSet(RF_PowerTypeDef RF_TxPowerLevel)
{
	unsigned char *p;
	if (RF_TxPowerLevel + 1 > (sizeof (TblRFPower)>>2)) {
		RF_TxPowerLevel = (sizeof (TblRFPower)>>2) - 1;
	}
	p = TblRFPower + RF_TxPowerLevel * 4;
	WriteAnalogReg (0xa2, *p ++);
	WriteAnalogReg (0x04, *p ++);
	WriteAnalogReg (0xa7, *p ++);
	WriteAnalogReg (0x8d, *p ++);
}

unsigned int RF_FsmIsIdle(void)
{
    return (REG_RF_FSM_STATUS == 0);
}


// just for debug

// set max receive length  // need check
//0x042c
void  RF_SetMaxRcvLen(unsigned char RF_MaxRxLen)
{
	REG_RF_RX_MAX_LEN = RF_MaxRxLen;
}

/**
*	@brief	 This function is to update TP(two point),this value will affect
*			 RF performance
*			 it needs to use  before the function  "RF_TrxStateSet"
*
*	@param[in]	RF_Mode  	Set working mode for RF module.
*	@param[in]	RF_TPLow  	Tp value for lower frequency (2400Mhz)
*							range: 2M/250K : 0x40(+/-)10   1M :0x1d(+/-)10
*							If you set a value outside the range, you will be set to fail.
*	@param[in]	RF_TPHigh  	Tp value for higher frequency (2480Mhz)
*							range: 2M/250K : 0x39(+/-)10   1M :0x19(+/-)10
*							If you set a value outside the range, you will be set to fail.
*
*	@return	 	0 :set success; 
*              -1 :set failed
*/
int RF_UpdateTpValue(RF_ModeTypeDef RF_Mode ,unsigned  char RF_TPLow,unsigned  char RF_TPHigh)
{
	if ((RF_Mode == RF_MODE_BLE_1M)||(RF_Mode == RF_MODE_STANDARD_BLE_1M)) {
		if( (RF_TPLow <= (aTPGain0[1] +10)) &&  (RF_TPLow >= (aTPGain0[1] - 10)) && \
			(RF_TPHigh <= (aTPGain1[1] +10)) &&  (RF_TPHigh >= (aTPGain1[1] - 10))){
			aTPGain0[1]=RF_TPLow;
			aTPGain1[1]=RF_TPHigh;
			aTPGain0[4]=RF_TPLow;
			aTPGain1[4]=RF_TPHigh;
			return 0;
		}

	}
	else {
		if(( RF_TPLow <= (aTPGain0[0] +10))&& ( RF_TPLow >= (aTPGain0[0] - 10)) && \
			(RF_TPHigh <= (aTPGain1[0] +10)) && ( RF_TPHigh >= (aTPGain1[0] - 10))){
			aTPGain0[0]=RF_TPLow;
			aTPGain1[0]=RF_TPHigh;
			aTPGain0[2]=RF_TPLow;
			aTPGain1[2]=RF_TPHigh;
			aTPGain0[3]=RF_TPLow;
			aTPGain1[3]=RF_TPHigh;
			return 0;
		}
	}
	return -1;

}

/**
*	@brief	   This function is to update cap value to affect RF performance
*			   it needs to use after the function  "RF_Init"
*
*	@param[in] RF_Cap - cap value : <4:0> is cap value(0x00 - 0x1f)
*
*	@return	   none
*/
void RF_UpdateCapValue(unsigned  char RF_Cap)
{
	//ana_81<4:0> is cap value(0x00 - 0x1f)
	WriteAnalogReg(REG_RF_FREOFFSET_CAP, (ReadAnalogReg(REG_RF_FREOFFSET_CAP)&0xe0) | (RF_Cap & 0x1f));
}

/**
*	@brief	  	This function is turn off tx and rx
*   @param      none
*	@return	 	none
*/
void RF_SetTxRxOff ()
{
	REG_RF_FUNCTION1_EN = 0x29;
	CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
}

/**
*	@brief	  	This function is to obtain the RSSI value of the current channel
*				Before using it, it is necessary to set the "RF_MODE_RX" by "RF_TrxStateSet"
*
*   @param      none
*	@return	 	RSSI value :  unit is dbm
*/
signed char RF_GetRssi(void)
{
	signed char Peak;
	unsigned char Gain;
	unsigned char GainTable[11]={10,15,21,27,33,39,46,50,56,62,62};
    while(1)
    {
    	 Peak = REG_RF_RX_PEAK & FLD_RF_RX_PEAK;
    	 Gain = REG_RF_RX_GAIN & FLD_RF_RX_GAIN;
    	 if (0x00 == Peak) {
    		 Peak = -44;
    		 break;
    	 } else if (0x20 == Peak) {
    		 Peak = -36;
    		 break;
    	 } else if (0x30 == Peak) {
    		 Peak = -28;
    		 break;
    	 } else if (0x38 == Peak) {
    		 Peak = -20;
    		 break;
    	 } else if (0x3c == Peak) {
    		 Peak = -12;
    		 break;
    	 } else if (0x3e == Peak) {
    		 Peak = -4;
    		 break;
    	 } else if (0x3f == Peak) {
    		 Peak = 4;
    		 break;
    	 }
	}
    return Peak - GainTable[Gain];
}

/**
*	@brief	  	This function is to start energy detect of the current channel for zigbee mode
*				Before using it, it is necessary to set the "RF_MODE_RX" by "RF_TrxStateSet"
*   @param      none
*	@return	 	none
*/
signed char RssiMax = -110;
void RF_EdDetect(void)
{
	signed char Rssi;
	WRITE_REG8(0x800426,0x00);//RXAUTO bit[1]LNA gain bit[2]MIX gain
	WRITE_REG8(0x80048a,0x02);//manual rx gain setting gain:max-23.95
	/*reduce gain, the power span of ED test is -75dbm to -35dbm in Zigbee spec,
	this setting the RSSI span is -82dbm to -34dbm*/
	Rssi = RF_GetRssi()+24;
    if (Rssi > RssiMax) {
    	RssiMax = Rssi;
    }
}


/**
*	@brief	  	This function is to stop energy detect and get energy detect value of
*				the current channel for zigbee mode.
*   @param      none
*	@return	 	ED:0x00~0xff
*
*/
unsigned char RF_StopEd(void)
{
	unsigned char RF_Ed;
	unsigned int Temp;
    /* Transfer the RSSI value to ED value */
    if (RssiMax <= -72) {
    	RF_Ed = 0;
    } else if (RssiMax >= -35) {
    	RF_Ed = 0xff;
    } else{
    	Temp = (RssiMax + 76) * 255;
    	RF_Ed = Temp/40;
    	if(RssiMax == -50)
    	{
    		RF_Ed = RF_Ed+2;
    	}
    }
    RssiMax = -110;
    WRITE_REG8(0x800426,0x07);//RXAUTO default value
    WRITE_REG8(0x80048a,0x0c);//manual rx gain setting default value
    return RF_Ed;
}
/**
*	@brief	  	This function is to stop energy detect and get energy detect value of
*				the current channel for zigbee mode.
*   @param      none
*	@return	 	ED:0x00~0xff
*
*/
int  RF_PowerDown(void)
{
	WriteAnalogReg (0x06,0xfe); 		//  power down analog module
}
/**
*	@brief	  	This function turns on power to the RF module
*   @param      none
*	@return	 	none
*
*/
int  RF_PowerOn(void)
{
	WriteAnalogReg (0x06,0x00); 		//  power on analog module
	WaitUs(5);//settle
}
