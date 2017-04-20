#include "../../drivers.h"
#include "../../common.h"

unsigned long firmwareVersion;

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
    PM_32kPadEnable();
    SYS_Init();

    GPIO_SetGPIOEnable(GPIOA_GP6, Bit_SET);
    GPIO_SetOutputEnable(GPIOA_GP6, Bit_RESET);

    unsigned int t0;

    while (1) {
        //pull-up GPIO pin
        GPIO_SetBit(GPIOA_GP6);
        GPIO_SetOutputEnable(GPIOA_GP6, Bit_SET);
        WaitMs(6000);

        t0 = ClockTime();
        PM_LowPwrEnter(DEEPSLEEP_MODE, WAKEUP_SRC_TIMER, t0 + 10*1000*1000*TickPerUs);
    }
}




