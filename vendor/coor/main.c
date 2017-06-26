#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/gateway.h"
#include "../../wsn/config.h"

#include "../../interface/pc_interface.h"

unsigned long firmwareVersion;

volatile unsigned char GatewaySetupTrig = 0;

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
    // WaitMs(1000);
}

volatile int test = 0;

void Board_Init(void)
{

    //config the setup trig GPIO pin
    GPIO_SetGPIOEnable(SW1_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(SW1_PIN, Bit_SET);   //enable input
    GPIO_PullSet(SW1_PIN, PULL_UP_1M);
    GPIO_SetInterrupt(SW1_PIN, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);

    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
    GPIO_SetOutputEnable(TIMING_SHOW_PIN, Bit_SET);

	//LED Pin
    GPIO_SetGPIOEnable(LED_GREEN, Bit_SET);
    GPIO_ResetBit(LED_GREEN);
    GPIO_SetOutputEnable(LED_GREEN, Bit_SET);

	//LED Pin
    GPIO_SetGPIOEnable(LED_RED, Bit_SET);
    GPIO_ResetBit(LED_RED);
    GPIO_SetOutputEnable(LED_RED, Bit_SET);


    GPIO_SetGPIOEnable(RX_STATE_PIN, Bit_SET);
    GPIO_ResetBit(RX_STATE_PIN);
    GPIO_SetOutputEnable(RX_STATE_PIN, Bit_SET);

    GPIO_SetGPIOEnable(TX_STATE_PIN, Bit_SET);
    GPIO_ResetBit(TX_STATE_PIN);
    GPIO_SetOutputEnable(TX_STATE_PIN, Bit_SET);
}

void Buff_Inface_Init(void)
{
	ParaBuf_Init((unsigned short)commandBuff,commandBuffSize,commandBuffCnt);
	ResuBuf_Init((unsigned short)resultBuff,resultBuffSize,resultBuffCnt);
}
void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    Board_Init();
    Gateway_Init();
    Buff_Inface_Init();
    IRQ_Enable();

    // LogMsg("gateway start...\n", NULL, 0);
    while (1)
    {

		test ++;
#if 1
		Gateway_MainLoop();
#endif
    }

}




