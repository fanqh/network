#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/node.h"
#include "../../wsn/config.h"

#define CHIP_ID    0x0101
#define LED_PIN    GPIOC_GP3

unsigned long firmwareVersion=8;

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

int test = 0;

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    Node_Init();
    //mesh network setup
    Node_SetupLoop();
    GPIO_SetBit(LED_PIN);
    GPIO_ResetBit(TIMING_SHOW_PIN);

    //WaitMs(1000);

    // LogMsg("end device start...\n", NULL, 0);

    while (1) {
    	test++;
        Node_MainLoop();
    }
}




