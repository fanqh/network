#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/pallet.h"
#include "../../wsn/config.h"

#define CHIP_ID                   0x0101
#define PALLET_SETUP_TRIG_PIN     GPIOD_GP2
#define LED_PIN                   GPIOC_GP3

unsigned long firmwareVersion;

volatile unsigned char PalletSetupTrig = 0;

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
    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
    // WaitMs(1000);

    GPIO_SetGPIOEnable(LED_PIN, Bit_SET);
    GPIO_ResetBit(LED_PIN);
    GPIO_SetOutputEnable(LED_PIN, Bit_SET);

}
static void Board_Init(void)
{
	//LED Pin
    GPIO_SetGPIOEnable(LED_PIN, Bit_SET);
    GPIO_ResetBit(LED_PIN);
    GPIO_SetOutputEnable(LED_PIN, Bit_SET);
    //config the setup trig GPIO pin
    GPIO_SetGPIOEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);   //enable input
    GPIO_PullSet(PALLET_SETUP_TRIG_PIN, PULL_UP_1M);
    GPIO_SetInterrupt(PALLET_SETUP_TRIG_PIN, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
    //for debug Pin
	GPIO_SetGPIOEnable(DEBUG_PIN, Bit_SET);
    GPIO_ResetBit(DEBUG_PIN);
    GPIO_SetOutputEnable(DEBUG_PIN, Bit_SET);
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    Board_Init();
    IRQ_Enable();
    while(!PalletSetupTrig); //wait for pallet setup trig

    Pallet_Init();
    Pallet_SetupLoop();
    GPIO_SetBit(LED_PIN);
    WaitMs(1000);
    GPIO_ResetBit(LED_PIN);
    Pallet_SetupLoop2();
    GPIO_SetBit(LED_PIN);
    GPIO_ResetBit(TIMING_SHOW_PIN);



    while (1) {
		firmwareVersion++;
        Pallet_MainLoop();
    }
}




