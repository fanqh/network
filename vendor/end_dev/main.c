#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/node.h"
#include "../../wsn/config.h"

#define CHIP_ID    0x0101
#define LED_PIN    GPIOC_GP3

unsigned long firmwareVersion=8;
#define TMP102A_ADDRESS 0x90

extern NodeInfo_TypeDef node_info;

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

    GPIO_SetGPIOEnable(DEBUG_PIN, Bit_SET);
    GPIO_ResetBit(DEBUG_PIN);
    GPIO_SetOutputEnable(DEBUG_PIN, Bit_SET);
}

int test = 0;


unsigned int Get_Temperature(void)
{
	unsigned int temperature;
	unsigned short tmp, tt, a1;

	I2C_read_bytes(0, (unsigned char *)&tmp, 2);

	if(tmp == 0xffff)
		return 0;
	tt = ((tmp>>8) | (tmp<<8));

	a1 = tt>>4;
	temperature = a1 * 625;

	return temperature;
}


unsigned int yy;
void main(void)
{

    PM_WakeupInit();
    SYS_Init();

    Node_Init();
    //mesh network setup
    Node_SetupLoop();
    GPIO_SetBit(LED_PIN);
    GPIO_ResetBit(TIMING_SHOW_PIN);

    I2C_PinSelect(I2C_PIN_GPIOB);
    I2C_Init(TMP102A_ADDRESS, 4);

    //WaitMs(1000);

    // LogMsg("end device start...\n", NULL, 0);

    while (1) {

//    	if((ClockTime() - test) >5000*200*TickPerUs)
//    	{
//    		test = ClockTime();
//    		node_info.tmp = Get_Temperature();
//    	}
    	//test++;
        Node_MainLoop();
    }
}




