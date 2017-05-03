/********************************************************************************************************
 * @file     bsp.c
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
#include "bsp.h"


#define REG_RST0        REG_ADDR8(0x60)
#define REG_RST1        REG_ADDR8(0x61)
#define REG_RST2        REG_ADDR8(0x62)
#define REG_CLK0_EN     REG_ADDR8(0x63)
#define REG_CLK1_EN     REG_ADDR8(0x64)
#define REG_CLK2_EN     REG_ADDR8(0x65)
#define REG_CLK_SEL     REG_ADDR8(0x66)
enum {
    FLD_CLK_SEL_DIV =    BIT_RNG(0,4),
    FLD_CLK_SEL_SRC =    BIT_RNG(5,6),
    FLD_CLD_SEL_FHS =    BIT(7),
};


#define REG_FHS_SEL    REG_ADDR8(0x70)
enum {
    FLD_FHS_SELECT =     BIT_RNG(0,1),
};

enum {
    FHS_SEL_192M_PLL = 0,
    FHS_SEL_32M_OSC = 2,
};
#define REG_CLK_MUX_SEL    REG_ADDR8(0x73)
enum {
    FLD_CLK_SP_LOW_POWER =    BIT_RNG(4,7),
};
#define REG_SYS_TIMER    REG_ADDR32(0x740)

#define REG_SYSTEM_TICK_CTRL    REG_ADDR8(0x74f)
enum {
    FLD_SYSTEM_TICK_START =    BIT(0),//?????
};


unsigned long TickPerUs;
static void SetTickUs (unsigned int t)
{
    TickPerUs = t;
}

#define REG_DMA_CHANNEL_INTERRUPT_MASK        REG_ADDR8(0x521)
void SysInit(BSP_SysCtlTypeDef *SysCtl)
{
    if (SysCtl != NULL) {
        REG_RST0 = SysCtl->rst0;
        REG_RST1 = SysCtl->rst1;
        REG_RST2 = SysCtl->rst2;
        REG_CLK0_EN = SysCtl->clk0;
        REG_CLK1_EN = SysCtl->clk1;
        REG_CLK2_EN = SysCtl->clk2;
    }
    //swires
    CLR_BIT_FLD(REG_RST1,FLD_RST_SWR);
    SET_BIT_FLD(REG_CLK1_EN,FLD_CLK_SWIRE_EN);

    //algm
    CLR_BIT_FLD(REG_RST1,FLD_RST_ALGM);
    SET_BIT_FLD(REG_CLK1_EN,FLD_CLK_ALGM_EN);

    //usb
    CLR_BIT_FLD(REG_RST0,FLD_RST_USB);
    SET_BIT_FLD(REG_CLK0_EN,FLD_CLK_USB_EN|FLD_CLK_USB_PHY_EN);

    WriteAnalogReg (0x88, 0x0f);
    WriteAnalogReg (0x05, 0x60);
    //system timer
    CLR_BIT_FLD(REG_RST1,FLD_RST_SYS_TIMER);
    SET_BIT_FLD(REG_CLK1_EN,FLD_CLK_SYS_TIMER_EN);
    CLR_BIT_FLD(REG_CLK_MUX_SEL,FLD_CLK_SP_LOW_POWER);//disable low power mode
    //clear DMA channel interrupt mask
    REG_DMA_CHANNEL_INTERRUPT_MASK = 0X00;
}

void SysClockInit(BSP_SysClkSelTypeDef ClockSrc, unsigned char ClockDivider)
{
    if (SYS_CLK_HS_DIV == ClockSrc) {
        //set FHS select and divider
        REG_FHS_SEL = FHS_SEL_192M_PLL;//FHS select
        REG_CLK_SEL = (0x20 | ClockDivider);
        SetTickUs(192/ClockDivider);
    }
    else if (SYS_CLK_RC == ClockSrc) {
        REG_CLK_SEL = 0x00;
        SetTickUs(32);
    }
    REG_SYSTEM_TICK_CTRL = FLD_SYSTEM_TICK_START; //sys timer beginning to work
}

void SysSetClockDivider(unsigned char ClockDivider)
{
    //set FHS select and divider
    WRITE_REG8(0x800070,0x00);//FHS select
    WRITE_REG8(0x800066,(0x20 | ClockDivider));
    SetTickUs(192/ClockDivider);
}

unsigned int ClockTime(void)
{
    return REG_SYS_TIMER;

}

unsigned int ClockTimeExceed(unsigned int Ref, unsigned int SpanUs)
{
    return ((unsigned int)(ClockTime() - Ref) > SpanUs * TickPerUs);
}

//-------------------Read/Write register---------------------

unsigned char ReadAnalogReg(unsigned char Addr)
{
    WRITE_REG8(0x8000b8,Addr);
    WRITE_REG8(0x8000ba,0x40);

    while((READ_REG8(0x8000ba) & 0x01) != 0x00);
    return READ_REG8(0x8000b9);
}
void WriteAnalogReg (unsigned char Addr, unsigned char Dat)
{
    WRITE_REG8(0x8000b8,Addr);
    WRITE_REG8(0x8000b9,Dat);
    WRITE_REG8(0x8000ba,0x60);
    while((READ_REG8(0x8000ba) & 0x01) != 0x00);
    WRITE_REG8(0x8000ba,0x00);
}
void SetChipId(unsigned short id)
{
    WRITE_REG8(0x800074, 0x53);
    WRITE_REG16(0x80007e, id);
    WRITE_REG8(0x800074, 0);
}

void PrintBufInit(unsigned char BufLen)
{
    WRITE_REG8(0x80013c, BufLen);
}

void WaitUs(int Microsec)
{
    unsigned int t = ClockTime();
    while (!ClockTimeExceed(t, Microsec)) {
    }
}
void WaitMs(int Millisec)
{
    while (Millisec) {
        WaitUs(1000);
        Millisec--;
    }
}

int LoadTblCmdSet(const BSP_TblCmdSetTypeDef * Table, int Size)
{
    int l=0;
    int loop;

    while (l < Size) {
        unsigned int  cadr = ADR_IO | Table[l].adr;
        unsigned char cdat = Table[l].dat;
        unsigned char ccmd = Table[l].cmd;
        unsigned char cvld = (ccmd & TCMD_UNDER_WR);
        ccmd &= TCMD_MASK;
        if (cvld) {
            if (ccmd == TCMD_WRITE) {

                WRITE_REG8 (cadr, cdat);
            }
            else if (ccmd == TCMD_WAREG) {

                WriteAnalogReg (cadr, cdat);
            }
            else if (ccmd == TCMD_WAIT) {

                WaitUs (Table[l].adr*256 + cdat);
            }
        }
        l++;
    }
    return Size;
}

void USB_DpPullUpEn(int En)
{
    unsigned char dat = ReadAnalogReg(0x08);
    if (En) {
        dat = (dat & 0x3f) | BIT(7);
    }
    else {
        dat = (dat & 0x3f) | BIT(6);
    }
    WriteAnalogReg (0x08, dat);
}

//log message
static void SendStatusToUsb(unsigned char *p, int n)
{
    int i;
    WRITE_REG8(0x800118, n); //reg_usb_ep8_dat = n;
    if (n) {     // print DATA
        for (i = 1; i < n+1; i++) {
            WRITE_REG8(0x800118, *p); //reg_usb_ep8_dat = *p++;
            p++;
        }
    }
    else {       //print ASCII
        for ( i = 1; i < 60; i++) {
            if (*p) {
                WRITE_REG8(0x800118, *p); //reg_usb_ep8_dat = *p++;
                p++;
            }
            else {
                //reg_usb_ep8_dat = 0;
                WRITE_REG8(0x800118, 0);
                break;
            }
        }
    }
    // reg_usb_ep8_ctrl = BIT(7);
    WRITE_REG8(0x800120, BIT(7));
}

void LogMsg(char *Str, unsigned char *Content, int Len)
{
    if (Str) {
        SendStatusToUsb(Str, 0);
    }
    if (Len > 32) {
        Len = 32;
    }
    if (Len > 0) {
        SendStatusToUsb(Content, Len);
    }
}

void USB_LogInit(void)
{
    WRITE_REG8(0x80013c, 0x40);
    WRITE_REG8(0x80013d, 0x09);
}

#define REG_RND_NUMBER          REG_ADDR16(0x448)
unsigned short Rand(void)
{
    return (unsigned short)((REG_SYS_TIMER & 0xffff) ^ REG_RND_NUMBER);
}



