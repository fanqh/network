#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/gateway.h"
#include "../../wsn/config.h"

#define GW_SETUP_TRIG_PIN    GPIOD_GP2
extern volatile unsigned char GatewaySetupTrig;

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    if (IrqSrc & FLD_IRQ_GPIO_EN) {
        if (0 == GPIO_ReadInputBit(GW_SETUP_TRIG_PIN)) {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(GW_SETUP_TRIG_PIN)) {
                while(0 == GPIO_ReadInputBit(GW_SETUP_TRIG_PIN));
                GatewaySetupTrig = 1;
            }
        } 
    }

    if (IrqSrc & FLD_IRQ_ZB_RT_EN) {
        if (RfIrqSrc) {
            if (RfIrqSrc & FLD_RF_IRQ_RX) {
                Gateway_RxIrqHandler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT) {
                Gateway_RxTimeoutHandler();
            }
#if PA_MODE
            if(RfIrqSrc & FLD_RF_IRQ_TX)
            {
            	Pa_Mode_Switch(PA_RX_MODE);
            }
#endif

            IRQ_RfIrqSrcClr();
        }
    }

    
    IRQ_SrcClr();
}



