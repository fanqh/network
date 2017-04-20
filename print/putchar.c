#ifndef WIN32
#define WIN32

//#include "../../platform/platform_includes.h"
//#include "putchar.h"
//#include "../drivers/usbhw.h"

#include "../drivers.h"
#include "../common/types.h"

#define UART_DEBUG 1

#if UART_DEBUG
int putchar(int c){
	
	UART_Send_Byte(c);
	return c;
//	if(UART_Send(c)==1)
//		return c;
//	else
//		return -1;

}
#else
#define USB_PRINT_TIMEOUT	 10		//  about 10us at 30MHz
#define USB_SWIRE_BUFF_SIZE  248	// 256 - 8

#define USB_EP_IN  		(USB_EDP_PRINTER_IN  & 0X07)	//  from the point of HOST 's view,  IN is the printf out
#define USB_EP_OUT  	(USB_EDP_PRINTER_OUT & 0X07)

int usb_putc(int c)
{
#if 1
	int i = 0;
	while(i ++ < USB_PRINT_TIMEOUT)
	{
		if(!(REG_USB_EP8_FIFO_MODE & FLD_USB_ENP8_FULL_FLAG))
		{
			REG_USB_EP_DAT(USB_EP_IN) = (u8)c;
			return c;
		}
	}
	return -1;
#endif

#if 0
	while (reg_usb_ep8_fifo_mode & FLD_USB_ENP8_FULL_FLAG);
	reg_usb_ep_dat(USB_EP_IN) = (u8)c;
	return c;
#endif
}

int putchar(int c)
{
	return usb_putc((char)c);
//	if(reg_usb_host_conn)
//	{
//		swire_is_init = 0;		// should re-init swire if connect swire again
//		return usb_putc((char)c);
//	}else
//	{
//		return swire_putc((char)c);
//	}
}
#endif

#endif

