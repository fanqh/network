#include "../../drivers.h"
#include "../../common.h"
#include "../../wsn/pallet.h"
#include "../../wsn/config.h"

#define CHIP_ID    0x0101

unsigned long firmwareVersion;
extern int my_printf(const char *format, ...);
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
    WaitMs(1000);
}

int test = 0;
extern void debug_enqueue(unsigned char type);
void main(void)
{


    PM_WakeupInit();
    SYS_Init();

    Pallet_Init();

    WaitMs(1000);
     //LogMsg("pallet start...\n", NULL, 0);
    while (1) {

        Pallet_MainLoop();
    	test ++;
    }
}




