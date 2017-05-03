#include "../../common.h"
#include "../../drivers.h"
#include "../../wsn/pallet.h"

#define PALLET_SETUP_TRIG_PIN     GPIOD_GP2
extern volatile unsigned char PalletSetupTrig;

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 IrqSrc = IRQ_SrcGet();
    u16 RfIrqSrc = IRQ_RfIrqSrcGet();

    if (IrqSrc & FLD_IRQ_GPIO_EN) {
        if (0 == GPIO_ReadInputBit(PALLET_SETUP_TRIG_PIN)) {
            WaitUs(10);
            if (0 == GPIO_ReadInputBit(PALLET_SETUP_TRIG_PIN)) {
                while(0 == GPIO_ReadInputBit(PALLET_SETUP_TRIG_PIN));
                PalletSetupTrig = 1;
            }
        } 
    }

    if (IrqSrc & FLD_IRQ_ZB_RT_EN) {
        if (RfIrqSrc) {
            if (RfIrqSrc & FLD_RF_IRQ_RX) {
                Pallet_RxIrqHandler();
            }
            
            if (RfIrqSrc & FLD_RF_IRQ_RX_TIMEOUT) {
                Pallet_RxTimeoutHandler();
            }

            IRQ_RfIrqSrcClr();
        }
    }
    
    IRQ_SrcClr();
}



