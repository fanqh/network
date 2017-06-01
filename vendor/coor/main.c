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
    GPIO_SetGPIOEnable(GW_SETUP_TRIG_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(GW_SETUP_TRIG_PIN, Bit_SET);   //enable input
    GPIO_PullSet(GW_SETUP_TRIG_PIN, PULL_UP_1M);
    GPIO_SetInterrupt(GW_SETUP_TRIG_PIN, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);
    //debug pin
    GPIO_SetGPIOEnable(DEBUG_PIN, Bit_SET);
    GPIO_ResetBit(DEBUG_PIN);
    GPIO_SetOutputEnable(DEBUG_PIN, Bit_SET);

    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
    GPIO_SetOutputEnable(TIMING_SHOW_PIN, Bit_SET);

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
#if 1
//    	if(GatewaySetupTrig != 0)
//    	{
//    		if(SetupInitFlag == 0)
//    		{
//    			 SetupInitFlag = 1;
//    			 Gateway_Init();
//    		}
//
//    		Gateway_SetupLoop();
//    	}
//    	else
    	{
			test ++;
			Gateway_MainLoop();
    	}
#endif
    }

}




