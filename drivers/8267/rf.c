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
	0x430,0x12, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x43d,0xb1, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x438,0xb7, TCMD_UNDER_BOTH | TCMD_WRITE,


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
	  0x042b, 0xf5, TCMD_UNDER_BOTH | TCMD_WRITE, // access code: 1
	  0x042c, 0x88, TCMD_UNDER_BOTH | TCMD_WRITE, // maxiumum length 48-byte
	
	  0x0464, 0x06, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x04cd, 0x00, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x0f03, 0x1e, TCMD_UNDER_BOTH | TCMD_WRITE, // bit3: crc2_en; normal 1e

	  //AGC_table
	  0x043a, 0x33, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x04c0, 0x81, TCMD_UNDER_BOTH | TCMD_WRITE,
	  
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

	0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
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

	0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
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

	0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,
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
	  0x043a, 0x22, TCMD_UNDER_BOTH | TCMD_WRITE,
	  0x04c0, 0x87, TCMD_UNDER_BOTH | TCMD_WRITE,

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



void RF_SetGainManualMax (void)
{

	CLR_BIT_FLD(REG_RF_RX_SATH, FLD_RF_MGAIN_EN);//  set mgain disable
	CLR_BIT_FLD(REG_RF_RX_PEL0, FLD_RF_RX_POW_THD_LOW);//  [6:4]pel0
	REG_RF_RX_PCHG =  0x77;//	Rx signal power change threshold
	REG_RF_RX_FGAIN = 0xc9;//  set rx peak detect manual
	if ((RF_MODE_BLE_1M ==g_RFMode)||(RF_MODE_BLE_2M ==g_RFMode)||(RF_MODE_STANDARD_BLE_1M ==g_RFMode)) {
		REG_RF_RET_RXPGA	= 0x06;//  [1:0]fix rst_pga=0 [2]len_0_en(ble)
	}
	else {
		REG_RF_RET_RXPGA = 0x02;//  [1:0]fix rst_pga=0
	}

}
void RF_SetAgc (void)
{
	SET_BIT_FLD(REG_RF_RX_SATH, FLD_RF_MGAIN_EN);//  set mgain enable
	SET_BIT_FLD(REG_RF_RX_PEL0, FLD_RF_RX_POW_THD_LOW);//  [6:4]pel0
	if ( RF_MODE_ZIGBEE_250K == g_RFMode) {
		REG_RF_RX_PCHG =  0x33;//	Rx signal power change threshold
	}
	else {
		REG_RF_RX_PCHG =  0x22;//	Rx signal power change threshold
	}
	REG_RF_RX_FGAIN = 0x20;
	if ((RF_MODE_BLE_1M == g_RFMode)||(RF_MODE_BLE_2M == g_RFMode)||(RF_MODE_STANDARD_BLE_1M == g_RFMode)) {
		REG_RF_RET_RXPGA = 0x04;  //[1:0]fix rst_pga=0 [2]len_0_en(ble)
	}
	else {
		REG_RF_RET_RXPGA = 0x00;//[1:0]fix rst_pga=0
	}
}

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



void RF_BaseBandReset (void)
{
	SET_BIT_FLD(REG_RST0, FLD_RST_ZB);
	CLR_BIT_FLD(REG_RST0, FLD_RST_ZB);
}


int  RF_TrxStateSet(RF_StatusTypeDef  RF_Status,signed char RF_Channel)
{
    int  err = 0;

	if (RF_Status == RF_MODE_TX) {
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL = 2400 + RF_Channel;
		SET_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN);
		CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	}
	else if (RF_Status == RF_MODE_RX) {
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL =  2400 + RF_Channel;
		SET_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_RX_MANUAL_EN);
		SET_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	}
	else if (RF_Status == RF_MODE_AUTO) {
		REG_RF_FUNCTION1_EN = 0x29;
		CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
		CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
		REG_RF_TX_CHANNEL = 2400 + RF_Channel;
	}
	else if (RF_Status == RF_MODE_OFF) {
	    WriteAnalogReg (0x06,0xff); 		//  power down analog module 
		WriteAnalogReg (0x05,0xe2); 		//  power down analog module
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

void RF_TxPkt (unsigned char* RF_TxAddr)
{
	REG_DMA_RF_TX_ADDR 	= (unsigned short)(RF_TxAddr);
	REG_DMA_RF_TX_RDY0 |= FLD_DMA_RF_TX;
}

void RF_StartStx  (unsigned char* RF_TxAddr,unsigned int RF_StartTick)
{

	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);// Enable cmd_schedule mode
	REG_RF_CMD_START_TICK = RF_StartTick;// Setting schedule trigger time
	REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
	SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_STX);// single TX
}
void RF_StartSrx(unsigned int RF_StartTick,unsigned int RF_RxTimeoutUs)
{
	REG_RF_RX_FIRST_TIMEOUT_US = RF_RxTimeoutUs-1;// first timeout
	REG_RF_CMD_START_TICK = RF_StartTick;		// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
	SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_SRX);// single rx
}
void RF_StartStxToRx  ( unsigned char* RF_TxAddr ,unsigned int RF_StartTick,unsigned short RF_RxTimeoutUs)
{
	REG_RF_RX_TIMEOUT_US = RF_RxTimeoutUs-1;
	REG_RF_CMD_START_TICK = RF_StartTick;// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
    REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
    SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_STR);// single tx2rx
}

void RF_StartSrxToTx  (unsigned char* RF_TxAddr  ,unsigned int RF_StartTick,unsigned int RF_RxTimeoutUs)
{
	REG_RF_RX_FIRST_TIMEOUT_US = RF_RxTimeoutUs-1;// first timeout
	REG_RF_CMD_START_TICK = RF_StartTick;		// Setting schedule trigger time
	SET_BIT_FLD(REG_RF_FUNCTION1_EN,FLD_RF_CMD_SCHEDULE_EN);
    REG_DMA_RF_TX_ADDR = (unsigned short)(RF_TxAddr);
    SET_BIT_FLD(REG_RF_FSM_CMD,FLD_FSM_RF_CMD_TRIGGER | FLD_FSM_RF_SRT);	// single rx2tx
}

void  RF_RxBufferSet(unsigned char *  RF_RxAddr, int Size, unsigned char  PingpongEn)
{
    unsigned char mode;
	mode = PingpongEn ? 0x03 : 0x01;
	REG_DMA_RF_RX_ADDR = (unsigned short)(RF_RxAddr);
	REG_DMA_RF_RX_BUF_SIZE = Size>>4;
	REG_DMA_RF_RX_BUF_PIPO = mode;
	g_RFRxPingpongEn = PingpongEn;
}

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

void  RF_RxBufferClearFlag(unsigned char  RF_RxBufferIdx)
{
	SET_BIT_FLD(REG_DMA_RF_RX_RDY(RF_RxBufferIdx) , FLD_DMA_RF_RX);
}

unsigned char  RF_TxFinish(void)
{
	return  (REG_RF_IRQ_STATUS & FLD_RF_IRQ_TX);
}

void RF_TxFinishClearFlag (void)
{
	SET_BIT_FLD(REG_RF_IRQ_STATUS , FLD_RF_IRQ_TX);
}

void RF_TxAccessCodeSelect (unsigned char RF_TxPipeIdx)
{
	CLR_BIT_FLD(REG_RF_TX_ACCESS_CODE_CHN ,FLD_TX_ACCESSZ_CODE_CHN);
	REG_RF_TX_ACCESS_CODE_CHN |= RF_TxPipeIdx;//Tx_Channel_man[2:0]

}
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

void RF_AccessCodeSetting2345 (unsigned char  RF_PipeIdx,unsigned char  Prefix)
{
	REG_RF_ACCESS_CODE2_5_prefix(RF_PipeIdx-2) = Prefix;//access_code 2~5
}

void RF_RxAccessCodeEnable (unsigned char RF_RxPipeSel)
{
	CLR_BIT_FLD(REG_RF_PIPE_RX_CHN_EN ,FLD_RF_RX_ACCESS_CODE_CHN_EN);
	REG_RF_PIPE_RX_CHN_EN |= RF_RxPipeSel;//rx_access_code_chn_en
}

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
/**********************************************************************************************************************
**********************************************************************************************************************/
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
void RF_UpdateCapValue(unsigned  char RF_Cap)
{
	//ana_81<4:0> is cap value(0x00 - 0x1f)
	WriteAnalogReg(REG_RF_FREOFFSET_CAP, (ReadAnalogReg(REG_RF_FREOFFSET_CAP)&0xe0) | (RF_Cap & 0x1f));
}
void RF_SetTxRxOff ()
{
	REG_RF_FUNCTION1_EN = 0x29;
	CLR_BIT_FLD(REG_RF_RX_MODE,FLD_RF_RX_ENABLE);
	CLR_BIT_FLD(REG_RF_FUNCTION0_EN,FLD_RF_TX_MANUAL_EN|FLD_RF_RX_MANUAL_EN);
}

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

signed char RssiMax = -110;
void RF_EdDetect(void)
{
	signed char Rssi;

	Rssi = RF_GetRssi();
    if (Rssi > RssiMax) {
    	RssiMax = Rssi;
    }
}

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
    return RF_Ed;
}

