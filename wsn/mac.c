/*
 * mac.c
 *
 *  Created on: 2017-6-16
 *      Author: Administrator
 */
#include "mac.h"

unsigned short Get_MAC_Addr(void)
{
	unsigned short addr;

	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(unsigned short), (unsigned char*)&addr);

	return addr;
}

unsigned int Estimate_SendT_From_RecT(unsigned int rec_time, unsigned char rec_size)
{

	return rec_time- 0x95*TickPerUs - 32*(rec_size+6);

}
