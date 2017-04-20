#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/gateway.h"

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    if (IrqSrc & FLD_IRQ_ZB_RT_EN) {
        if (RfIrqSrc) {
            if (RfIrqSrc & FLD_RF_IRQ_RX) {
                Gateway_RxIrqHandler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT) {
                Gateway_RxTimeoutHandler();
            }

            IRQ_RfIrqSrcClr();
        }
    }
    
    IRQ_SrcClr();
}



