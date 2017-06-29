#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/gateway.h"
#include "../../wsn/config.h"


extern volatile unsigned char GatewaySetupTrig;

typedef struct
{
	unsigned not_first;
	unsigned int pre_timestamp;
}key_TypeDef;

key_TypeDef current_key, key;
_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();


    if (IrqSrc & FLD_IRQ_GPIO_EN)
    {
        if (0 == GPIO_ReadInputBit(SW1_PIN))
        {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(SW1_PIN))
            {
            	if(key.not_first !=0)
            	{
					if(ClockTime() - (key.pre_timestamp+300000*TickPerUs) <=BIT(31))
					{
						GatewaySetupTrig = 1;
						GPIO_WriteBit(LED_RED, !GPIO_ReadOutputBit(LED_RED));
					}
            	}
            	else
            	{
            		GatewaySetupTrig = 1;
            		key.not_first = 1;
            	}
                key.pre_timestamp = ClockTime();
            }
        } 
    }

    if (IrqSrc & FLD_IRQ_ZB_RT_EN)
    {

        if (RfIrqSrc)
        {
            if (RfIrqSrc & FLD_RF_IRQ_RX)
            {
                Gateway_RxIrqHandler();
            }
            
//            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT)
//            {
//                Gateway_RxTimeoutHandler();
//            }
//            if(RfIrqSrc & FLD_RF_IRQ_FIRST_TIMEOUT)
//            {
//            	Gateway_RxTimeoutHandler();
//            }
            if(RfIrqSrc & FLD_RF_IRQ_TX)
            {
            	Gateway_TxDoneHandle();
            }
#if PA_MODE
            PA_Auto_Switch_Next_State();
//            if(RF_TxFinish())
//            	Tx_Done_falg = 1;
#endif
            IRQ_RfIrqSrcClr();
        }
    }

    
    IRQ_SrcClr();
}



