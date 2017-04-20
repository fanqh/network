#include "../../drivers.h"
#include "../../common.h"

unsigned long firmwareVersion;

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
    WaitMs(1000);
}

static void adc_config(enum ADCINPUTCH chn,enum ADCINPUTMODE mode,enum ADCRFV ref_vol,enum ADCRESOLUTION resolution,enum ADCST sample_cycle){
    /***1.set the analog input pin***/
    adc_AnaChSet(chn);

    /***2.set ADC mode,signle-end or differential mode***/
    adc_AnaModeSet(mode);///default is single-end

    /***3.set reference voltage***/
    adc_RefVoltageSet(ref_vol);

    /***4.set resolution***/
    adc_ResSet(resolution);

    /***5.set sample cycle**/
    adc_SampleTimeSet(sample_cycle);
}

#define ADC_SIZE 16
static unsigned short battery_value[ADC_SIZE] = {0x00};
static unsigned int Battery_voltage = 0;

static volatile unsigned char rxBuf[128] = {0};
static unsigned char txBuf[128] = {0};
volatile unsigned char rx_irq_occur = 0;
volatile unsigned char tx_irq_occur = 0;
void main(void)
{
    /***1.init system clock,enable module clock,clear module reset**********/
    PM_WakeupInit();
    SYS_Init();

    /***2.init adc***/
    adc_Init();
    //config adc for battery detection
    adc_BatteryCheckInit(1);

    //config UART for printing log message
    UART_RecBuffInit(rxBuf, sizeof(rxBuf));
    UART_GPIO_CFG_PB2_PB3();
    UART_Init(921600, PARITY_NONE, STOP_BIT_ONE);
    IRQ_UartIrqEnable(FLD_UART_IRQ_RX | FLD_UART_IRQ_TX);
    IRQ_EnableType(FLD_IRQ_DMA_EN);
    IRQ_Enable();

    int i = 0;
    static unsigned int sum = 0;
    while (1) {
        //do average filtering
        for (i = 0; i < ADC_SIZE; i++) {
            battery_value[i] = adc_SampleValueGet();//get battery ADC value
            battery_value[i] &= 0x3fff; //resolution is 14 bit
            sum += battery_value[i];
        }
        sum /= ADC_SIZE;
        
        /*As for the ADC module, the indeed relation between digital output 
          and the analog input isn't exactly linear. When analog input is zero
          , the digital output is 128 and when analog input is equal to Vref, 
          the output is 2^14-128. So Vin = Vref*(Dout-128)*64/63/(2^14).
          */
        //battery voltage = 3*Vref*(adc_output-128)*64/63/2^14
        Battery_voltage = 3*1428*(sum -128)*64/63/16384;
        sum = 0;

        //print battery voltage on PC via UART
        txBuf[0] = sizeof(Battery_voltage);
        txBuf[1] = 0;
        txBuf[2] = 0;
        txBuf[3] = 0;
        memcpy(&txBuf[4], &Battery_voltage, sizeof(Battery_voltage));
        while (!UART_Send(txBuf));
        while(!tx_irq_occur);
        tx_irq_occur = 0;
        WaitMs(1000); //this delay is unnecessary and just for printing log message clearly
    }
}



