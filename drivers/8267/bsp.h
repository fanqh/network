/********************************************************************************************************
 * @file     bsp.h
 *
 * @brief    This file provides set of common functions for driver
 *
 * @author   kaixin.chen@telink-semi.com; qiuwei.chen@telink-semi.com
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
#ifndef BSP_H
#define BSP_H

/****************************************************
 	 	 	 	 Reg operations
 *****************************************************/

#define IOBASE                          0x800000
#define WRITE_REG32(addr,data)          (*(volatile unsigned long  *)(addr|IOBASE)=data)
#define WRITE_REG16(addr,data)          (*(volatile unsigned short *)(addr|IOBASE)=data)
#define WRITE_REG8(addr,data)           (*(volatile unsigned char  *)(addr|IOBASE)=data)

#define READ_REG32(addr)                (*(volatile unsigned long  *)(addr|IOBASE))
#define READ_REG16(addr)                (*(volatile unsigned short *)(addr|IOBASE))
#define READ_REG8(addr)                 (*(volatile unsigned char  *)(addr|IOBASE))

#define REG_ADDR8(a)                    (*(volatile unsigned char*) (IOBASE + (a)))
#define REG_ADDR16(a)                   (*(volatile unsigned short*)(IOBASE + (a)))
#define REG_ADDR32(a)                   (*(volatile unsigned int*)(IOBASE + (a)))

/****************************************************
 	 	 	 	Bit operations
 *****************************************************/

#define BIT(n)                  ( 1<<(n) )
#define BIT_MASK_LEN(len)       (BIT(len)-1)
#define BIT_RNG(s, e)  			(BIT_MASK_LEN((e)-(s)+1) << (s))

#define     SETB(v,n)   (*(volatile unsigned char  *)v |= n)
#define     CLRB(v,n)   (*(volatile unsigned char  *)v &= ~n)
#define     TEST(v,n)   (((*(volatile unsigned char  *)v) & n) ? 1:0)


#define BIT_SET(x, mask)         ( (x) |= (mask) )
#define BIT_CLR(x, mask)         ( (x) &= ~(mask) )
#define BIT_IS_SET(x, mask)      ( (x) & (mask) )
#define BIT_IS_CLR(x, mask)      ( (~x) & (mask) )
#define BIT_FLIP(x, mask)        ( (x) ^=  (mask) )

#define SET_BIT_FLD(x, mask)     BIT_SET(x, mask)
#define CLR_BIT_FLD(x, mask)     BIT_CLR(x, mask)
#define FLIP_BIT_FLD(x, mask)    BIT_FLIP(x, mask)


/****************************************************

 *****************************************************/

#define NULL        0
#define	ADR_IO		0x800000
#define TCMD_UNDER_RD		0x80
#define TCMD_UNDER_WR		0x40
#define TCMD_UNDER_BOTH		0xc0
#define TCMD_MASK		0x3f

#define TCMD_WRITE		0x3
#define TCMD_WAIT		0x7
#define TCMD_WAREG		0x8


typedef struct {
	unsigned short	adr;
	unsigned char	dat;
	unsigned char	cmd;
} BSP_TblCmdSetTypeDef;

typedef struct {
	unsigned char 	 rst0;
	unsigned char    rst1;
	unsigned char    rst2;
	unsigned char    clk0;
	unsigned char 	 clk1;
	unsigned char    clk2;
	unsigned char    clkctl;
} BSP_SysCtlTypeDef;

//rst0
enum {
	FLD_RST_SPI = 				BIT(0),
	FLD_RST_I2C = 				BIT(1),
	FLD_RST_USB = 				BIT(2),
	//FLD_RST_USB_PHY = 			BIT(3),
	FLD_RST_MCU = 				BIT(4),
	FLD_RST_MAC =				BIT(5),
	FLD_RST_AIF = 				BIT(6),
	FLD_RST_BB = 				BIT(7),
	FLD_RST_ZB =				BIT(7),
	FLD_RST0_ALL =				0xff,
};

//rst1
enum {
	FLD_RST_SYS_TIMER = 		BIT(0),
	FLD_RST_ALGM = 				BIT(1),
	FLD_RST_DMA =				BIT(2),
	FLD_RST_UART = 				BIT(3),
	FLD_RST_PWM = 				BIT(4),
	FLD_RST_AES = 				BIT(5),
	FLD_RST_BBPLL48M =			BIT(6),
	FLD_RST_SWR =				BIT(7),
	FLD_RST1_ALL =				0xff,
};
//rst2
enum {
	FLD_RST_SBC =				BIT(0),
	FLD_RST_AUD =				BIT(1),
	FLD_RST_DFIFO =				BIT(2),
	FLD_RST_ADC =				BIT(3),
	FLD_RST_MCIC =				BIT(4),
	FLD_RST_SOFT_MCIC = 		BIT(5),
	FLD_RST_MSPI =				BIT(6),
	FLD_RST_ALGS =				BIT(7),
	FLD_RST2_ALL =				0xff,
};
//clk0
enum {
	FLD_CLK_SPI_EN =			BIT(0),
	FLD_CLK_I2C_EN =			BIT(1),
	FLD_CLK_USB_EN =			BIT(2),
	FLD_CLK_USB_PHY_EN =		BIT(3),
	FLD_CLK_MCU_EN =			BIT(4),
	FLD_CLK_MAC_EN =			BIT(5),
	FLD_CLK_AIF_EN =			BIT(6),
	FLD_CLK_ZB_EN =				BIT(7),
	FLD_CLK0_EN_ALL =			0xff,
};

//clk1
enum {
	FLD_CLK_SYS_TIMER_EN = 		BIT(0),
	FLD_CLK_ALGM_EN = 			BIT(1),
	FLD_CLK_DMA_EN = 			BIT(2),
	FLD_CLK_UART_EN = 			BIT(3),
	FLD_CLK_PWM_EN = 			BIT(4),
	FLD_CLK_AES_EN = 			BIT(5),
	FLD_CLK_32K_TIMER_EN =		BIT(6),
	FLD_CLK_SWIRE_EN = 			BIT(7),
	FLD_CLK1_EN_ALL =			0xff,
};
//clk2
enum {
	FLD_CLK_32K_QDEC_EN =		BIT(0),
	FLD_CLK_AUD_EN =			BIT(1),
	FLD_CLK_DIFIO_EN = 			BIT(2),
	FLD_CLK_KEYSCAN_EN =		BIT(3),
	FLD_CLK_MCIC_EN =			BIT(4),
	FLD_CLK_QDEC_EN =			BIT(5),
	FLD_CLK_32K_PWM_EN = 		BIT(6),
	FLD_CLK_32K_KEYSCAN_EN =	BIT(7),
	FLD_CLK2_EN_ALL =			0xff,
};


typedef enum {
    SYS_CLK_RC = 0,
    SYS_CLK_HS_DIV = 1,
} BSP_SysClkSelTypeDef;

extern unsigned long TickPerUs;

/**
 * @brief This function initializes the clock of each peripheral module of MCU
 * @param[in]   SysCtl the pointer to the value of each peripheral module of MCU
 * @return none
 */
extern void SysInit(BSP_SysCtlTypeDef *SysCtl);
/**
 * @brief This function initializes  the system clock
 * @param[in]   ClockSrc the clock source of the system clock
 * @param[in]   ClockDivider the division factor of system clock if FHS is selected as its clock
 *              source, i.e., system clock = FHS / clock_divider
 * @return none
 */
extern void SysClockInit(BSP_SysClkSelTypeDef ClockSrc, unsigned char ClockDivider);
/**
 * @brief This function sets the division factor of system clock if FHS is selected as its clock
 * @param[in]   ClockDivider the division factor of system clock if FHS is selected as its clock
 *              source, i.e., system clock = FHS / clock_divider
 * @return none
 */
extern void SysSetClockDivider(unsigned char ClockDivider);
/**
 * @brief This function gets the current tick count of system timer
 * @return the current tick count of system timer
 */
extern unsigned int ClockTime(void);
/**
 * @brief This function determines whether current moment exceeds the reference moment plus
 *        the span time
 * @param[in]   Ref the moment used as reference
 * @param[in]   SpanUs the span time
 * @return whether current moment exceeds the reference moment plus the span time(1: exceed 0: no exceed)
 */
extern unsigned int ClockTimeExceed(unsigned int Ref, unsigned int SpanUs);

/**
 * @brief This function delays for the specified number of microseconds
 * @param[in]   Millisec the number of microseconds
 * @return none
 */
void WaitUs(int Microsec);
/**
 * @brief This function delays for the specified number of milliseconds
 * @param[in]   Millisec the number of milliseconds
 * @return none
 */
void WaitMs(int Millisec);
//-------------------Read/Write register---------------------
/**
 * @brief This function reads a byte data fromn a specified analog register
 * @param[in]   Addr the address of the analog register needs to read
 * @return the content of the analog register needs to read
 */
unsigned char ReadAnalogReg(unsigned char Addr);
/**
 * @brief This function writes a byte data to a specified analog register
 * @param[in]   Addr the address of the analog register needs to write
 * @param[in]   Dat the data will be written to the analog register
 * @return none
 */
void WriteAnalogReg (unsigned char Addr, unsigned char Dat);
/**
 * @brief This function performs a series of operations of writing digital or analog registers
 *        according to a command table
 * @param[in]   Table pointer to a command table containing several writing commands
 * @param[in]   Size number of commands in the table
 * @return number of commands are carried out
 */
extern int LoadTblCmdSet(const BSP_TblCmdSetTypeDef * Table, int Size);

/**
 * @brief This function enables or disables the internal pull-up resistor
 *        of DP pin of USB interface
 * @param[in]   En enables or disables the internal pull-up resistor(1: enable 0: disable)
 * @return none
 */
extern void USB_DpPullUpEn(int En);
/**
 * @brief This function set the two-byte ID for the chip
 * @param[in]   id the two-byte ID
 * @return none
 */
extern void SetChipId(unsigned short Id);
/**
 * @brief This function set the length of buffer in hardware for USB print
 * @param[in]   BufLen the length of buffer in hardware
 * @return none
 */
extern void PrintBufInit(unsigned char BufLen);
/**
 * @brief This function serves to print the log message via USB interface
 * @param[in]   Str pointer to the prompting string of the log message
 * @param[in]   Content pointer to the actual content of the log message
 * @return none
 */
extern void LogMsg(char *Str, unsigned char *Content, int Len);

/**
 * @brief This function initializes the USB interface for printing log message
 * @return none
 */
extern void USB_LogInit(void);

unsigned int time_elapsed( unsigned int sttk, unsigned int tm);

#endif /* BSP_H */
