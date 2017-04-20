#include "../../drivers.h"
#include "../../common.h"
#include "usb_desc.h"

unsigned long  firmwareVersion;

//define rx buffer
#define RX_BUF_LEN    64 //in bytes
#define RX_BUF_NUM    4
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM];
static unsigned char rx_ptr = 0;

//define rx callback function
static void USBCDC_RxCb(unsigned char *data)
{
    //set next rx buffer
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    USBCDC_RxBufSet(rx_buf + rx_ptr*RX_BUF_LEN);

    //process received data
    if (USBCDC_IsAvailable()) {
        USBCDC_DataSend(data, strlen(data));
        memset(data, 0, RX_BUF_LEN);
    }
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
    SysClockInit(SYS_CLK_HS_DIV, 6);
    USB_Init();
    USB_LogInit();
    USB_DpPullUpEn(1); //pull up DP pin of USB interface
}

void main(void)
{
    PM_WakeupInit();
    SYS_Init();

    USBCDC_RxBufSet(rx_buf + rx_ptr*RX_BUF_LEN);
    USBCDC_CBSet(USBCDC_RxCb, NULL);

    while (1) {
        ev_process_timer();
        USB_IrqHandle();
    }
}
