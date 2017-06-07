#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/gateway.h"
#include "../../wsn/config.h"


extern volatile unsigned char GatewaySetupTrig;

unsigned char sta[32];

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    GPIO_SetBit(LED_GREEN);
    if (IrqSrc & FLD_IRQ_GPIO_EN)
    {


        if (0 == GPIO_ReadInputBit(SW1_PIN))
        {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(SW1_PIN))
            {
                while(0 == GPIO_ReadInputBit(SW1_PIN));
                GatewaySetupTrig = 1;
            }
        } 
    }

    if (IrqSrc & FLD_IRQ_ZB_RT_EN)
    {

    	RF_FSM_STATE rf_current_state;

        if (RfIrqSrc)
        {
            if (RfIrqSrc & FLD_RF_IRQ_RX)
            {
                Gateway_RxIrqHandler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT)
            {
                Gateway_RxTimeoutHandler();
            }
#if PA_MODE
            PA_Auto_Switch_Next_State();
#endif
            IRQ_RfIrqSrcClr();
        }
    }

    
    IRQ_SrcClr();
}



