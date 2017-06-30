#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/pallet.h"
#include "../../wsn/config.h"


extern volatile unsigned char PalletSetupTrig;
typedef struct
{
	unsigned not_first;
	unsigned int pre_timestamp;
}key_TypeDef;

key_TypeDef key;

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();


    if (IrqSrc & FLD_IRQ_GPIO_EN)
    {
        if (0 == GPIO_ReadInputBit(PALLET_SETUP_TRIG_PIN))
        {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(PALLET_SETUP_TRIG_PIN))
            {

            	if(key.not_first !=0)
            	{
            		if(ClockTime() - (key.pre_timestamp+300000*TickPerUs) <=BIT(31))
					{
            			PalletSetupTrig = 1;
					}
            	}
            	else
            	{
            		PalletSetupTrig = 1;
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
                Pallet_RxIrqHandler();
                //IRQ_INDICATION();
            }
            if(RfIrqSrc & FLD_RF_IRQ_TX)
            {
            	Pallet_TxDoneHandle();
            }
            IRQ_RfIrqSrcClr();
        }

    }

    IRQ_SrcClr();
}



