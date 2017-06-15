#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/node.h"
#include "../../wsn/config.h"

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


    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
    GPIO_SetOutputEnable(TIMING_SHOW_PIN, Bit_SET);

    GPIO_SetGPIOEnable(LED1_GREEN, Bit_SET);
    GPIO_ResetBit(LED1_GREEN);
    GPIO_SetOutputEnable(LED1_GREEN, Bit_SET);

//    GPIO_SetGPIOEnable(LED2_BLUE, Bit_SET);
//    GPIO_ResetBit(LED2_BLUE);
//    GPIO_SetOutputEnable(LED2_BLUE, Bit_SET);

    GPIO_SetGPIOEnable(LED3_RED, Bit_SET);
    GPIO_ResetBit(LED3_RED);
    GPIO_SetOutputEnable(LED3_RED, Bit_SET);

    GPIO_SetGPIOEnable(LED4_WHITE, Bit_SET);
    GPIO_ResetBit(LED4_WHITE);
    GPIO_SetOutputEnable(LED4_WHITE, Bit_SET);
}

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
void Board_Init(void)
{

    //config the setup trig GPIO pin
    GPIO_SetGPIOEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);    //set as gpio
    GPIO_SetInputEnable(PALLET_SETUP_TRIG_PIN, Bit_SET);   //enable input
    GPIO_PullSet(PALLET_SETUP_TRIG_PIN, PULL_UP_1M);
    GPIO_SetInterrupt(PALLET_SETUP_TRIG_PIN, Bit_SET);
    IRQ_EnableType(FLD_IRQ_GPIO_EN);

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
}

void main(void)
{
	static unsigned int t;

	PM_WakeupInit();
    SYS_Init();

    Board_Init();
    Node_Init();
#if 0
    //mesh network setup
    Node_SetupLoop();
    GPIO_SetBit(LED1_GREEN);
    GPIO_ResetBit(TIMING_SHOW_PIN);

    I2C_PinSelect(I2C_PIN_GPIOA);
    I2C_Init(TMP102A_ADDRESS, 4);
    // LogMsg("end device start...\n", NULL, 0);
    t = ClockTime();
#endif
    while (1)
    {
        Node_MainLoop();
    }
}




