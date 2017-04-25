#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/pallet.h"

int volatile isr_cnt = 0;
_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    ++isr_cnt;

    if (IrqSrc & FLD_IRQ_ZB_RT_EN)
    {
        if (RfIrqSrc)
        {
            if (RfIrqSrc & FLD_RF_IRQ_RX)
            {
            	if(isr_cnt>0)
            		isr_cnt--;
                Pallet_RxIrqHandler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT)
            {
            	if(isr_cnt>0)
            		isr_cnt--;
                Pallet_RxTimeoutHandler();
            }

            IRQ_RfIrqSrcClr();
        }
    }
    
    IRQ_SrcClr();
}



