#include "../../drivers.h"
#include "../../common.h"

extern unsigned char spi_read_flag;
extern unsigned char spi_write_flag;

_attribute_ram_code_ __attribute__((optimize("-Os"))) void irq_handler(void)
{
    u32 src = irq_get_src();

    if (src & FLD_IRQ_HOST_CMD_EN) {
        u8 spi_irq_src = spi_irq_get_src();

        if (spi_irq_src & FLD_SPI_IRQ_HOST_CMD) {
            // if (spi_irq_src & FLD_SPI_IRQ_HOST_RD_TAG) {
            //     spi_read_flag = 1;
            // }
            // else {
                spi_write_flag = 1;
            // }
        }
        
        spi_irq_clr_src();
    }
}



