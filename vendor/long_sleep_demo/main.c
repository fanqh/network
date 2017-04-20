#include "../../drivers.h"
#include "../../common.h"

unsigned long firmwareVersion;

#define WAKE_UP_PIN_1    GPIOD_GP2
#define WAKE_UP_PIN_2    GPIOD_GP4
#define NOTIFY_PIN       GPIOD_GP3
#define LED2_PIN         GPIOD_GP6
#define LED4_PIN         GPIOD_GP5
#define LED3_PIN         GPIOD_GP7

static void PrepareSleep(void)
{
    //This function disable all unecessary GPIOs avoiding current leakage
    //disable OEN
    GPIO_SetOutputEnable(GPIOA_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOB_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOC_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOD_ALL, Bit_RESET);
    GPIO_SetOutputEnable(GPIOE_ALL, Bit_RESET);

    //disable dataO
    GPIO_ResetBit(GPIOA_ALL);
    GPIO_ResetBit(GPIOB_ALL);
    GPIO_ResetBit(GPIOC_ALL);
    GPIO_ResetBit(GPIOD_ALL);
    GPIO_ResetBit(GPIOE_ALL);

    //disable input_en
    GPIO_SetInputEnable(GPIOA_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOB_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOC_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOD_ALL, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP0, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP1, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP2, Bit_RESET);
    GPIO_SetInputEnable(GPIOE_GP3, Bit_RESET);
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
    SysClockInit(SYS_CLK_HS_DIV, 4);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    WaitMs(1000);
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    WaitMs(3000);

    while (1) {
        GPIO_SetGPIOEnable(LED2_PIN, Bit_SET);
        GPIO_SetOutputEnable(LED2_PIN, Bit_SET);
        GPIO_SetBit(LED2_PIN);
        WaitMs(100);
        GPIO_ResetBit(LED2_PIN);
        WaitMs(100);
        GPIO_SetBit(LED2_PIN);
        WaitMs(100);
        GPIO_ResetBit(LED2_PIN);
        WaitMs(100);

        PrepareSleep();
        // PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, ClockTime() + 3*1000*1000*TickPerUs);
        PM_LowPwrEnter2(SUSPEND_MODE, WAKEUP_SRC_TIMER, 5*1000*1000);
    } 
}




