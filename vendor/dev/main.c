#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/pallet.h"
#include "../../wsn/config.h"



unsigned long firmwareVersion;

extern PalletInfo_TypeDef pallet_info;

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

}

static void PrepareSleep(void)
{
    //Turn off all unecessary IOs before entering suspend to avoid current
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
void Board_Init(void)
{
    //config the setup trig GPIO pin
    GPIO_SetGPIOEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);   //enable input
    GPIO_PullSet(PALLET_SETUP_TRIG_PIN, PULL_UP_1M);
    GPIO_SetInterrupt(PALLET_SETUP_TRIG_PIN, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
#if 0
	GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
    GPIO_SetOutputEnable(TIMING_SHOW_PIN, Bit_SET);

	GPIO_SetGPIOEnable(POWER_PIN, Bit_SET);
    GPIO_ResetBit(POWER_PIN);
    GPIO_SetOutputEnable(POWER_PIN, Bit_SET);

	//LED Pin
    GPIO_SetGPIOEnable(LED1_GREEN, Bit_SET);
    GPIO_ResetBit(LED1_GREEN);
    GPIO_SetOutputEnable(LED1_GREEN, Bit_SET);

	GPIO_SetGPIOEnable(LED2_BLUE, Bit_SET);
    GPIO_ResetBit(LED2_BLUE);
    GPIO_SetOutputEnable(LED2_BLUE, Bit_SET);
    //GPIO_SetBit(LED2_BLUE);

	GPIO_SetGPIOEnable(LED3_RED, Bit_SET);
    GPIO_ResetBit(LED3_RED);
    GPIO_SetOutputEnable(LED3_RED, Bit_SET);

    //for debug Pin
	GPIO_SetGPIOEnable(LED4_WHITE, Bit_SET);
    GPIO_ResetBit(LED4_WHITE);
    GPIO_SetOutputEnable(LED4_WHITE, Bit_SET);

    //config gpio showing timing
    GPIO_SetGPIOEnable(SHOW_DEBUG, Bit_SET);
    GPIO_ResetBit(SHOW_DEBUG);
    GPIO_SetOutputEnable(SHOW_DEBUG, Bit_SET);

    //config gpio showing timing
    GPIO_SetGPIOEnable(TEST_PIN, Bit_SET);
    GPIO_ResetBit(TEST_PIN);
    GPIO_SetOutputEnable(TEST_PIN, Bit_SET);
#endif
}

//#define BUG
void main(void)
{
	PM_WakeupInit();
    SYS_Init();

    PrepareSleep();
    Board_Init();
    Pallet_Init();
    IRQ_Enable();

    while (1)
    {
         Pallet_MainLoop();
    }
}




