#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/node.h"
#include "../../wsn/config.h"

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    //IRQ_INDICATION();
    if (IrqSrc & FLD_IRQ_ZB_RT_EN)
    {
        if (RfIrqSrc) {
            if (RfIrqSrc & FLD_RF_IRQ_RX)
            {
                Node_RxIrqHandler();
            }
            
            if(RfIrqSrc & FLD_RF_IRQ_TX)
            {
            	Node_TxDoneHandle();
            }
            IRQ_RfIrqSrcClr();
        }
    }
    
    IRQ_SrcClr();
}



