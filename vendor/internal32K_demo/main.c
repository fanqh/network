#include "../../drivers.h"
#include "../../common.h"

unsigned long firmwareVersion;

static void prepare_to_sleep(void)
{
    //This function disable all unecessary GPIOs avoiding current leakage
    
    //diable OEN
    write_reg8(0x800582,0xff);
    write_reg8(0x80058a,0xff);
    write_reg8(0x800592,0xff);
    write_reg8(0x80059a,0xff);
    write_reg8(0x8005a2,0xff);

    //disable dataO
    write_reg8(0x800583,0x00);
    write_reg8(0x80058b,0x00);
    write_reg8(0x800593,0x00);
    write_reg8(0x80059b,0x00);
    write_reg8(0x8005a3,0x00);

    //diable input_en
    write_reg8(0x800581,0x00);
    write_reg8(0x800589,0x00);
    write_reg8(0x800591,0x00);
    write_reg8(0x800599,0x00);
    write_reg8(0x8005a1, read_reg8(0x8005a1) & 0xf0);
    //disable the input of DM/DP pins for eliminating the leakage current
    USB_DpPullUpEn(0);
}

static void SYS_Init(void)
{
    BSP_SysCtlTypeDef SysCtrl;
    SysCtrl.rst0 = (~FLD_RST0_ALL);
    SysCtrl.rst1 = (~FLD_RST1_ALL);
    SysCtrl.rst2 = (~FLD_RST2_ALL);
    SysCtrl.clk0 = FLD_CLK0_EN_ALL;
    SysCtrl.clk1 = FLD_CLK1_EN_ALL;
    SysCtrl.clk2 = FLD_CLK2_EN_ALL;
    SysInit(&SysCtrl);
    SysClockInit(SYS_CLK_HS_DIV, 6);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(1000);
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    GPIO_SetGPIOEnable(GPIOA_GP6, Bit_SET);
    GPIO_SetOutputEnable(GPIOA_GP6, Bit_RESET);

    unsigned int t0;

    while (1) {
        //pull-up GPIO pin
        GPIO_SetBit(GPIOA_GP6);
        GPIO_SetOutputEnable(GPIOA_GP6, Bit_SET);
        WaitMs(3000);

        prepare_to_sleep();
        t0 = ClockTime();
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, t0 + 10*1000*1000*TickPerUs);
    }
}




