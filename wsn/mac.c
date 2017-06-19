/*
 * mac.c
 *
 *  Created on: 2017-6-16
 *      Author: Administrator
 */
#include "mac.h"

extern unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4)));



unsigned short Get_MAC_Addr(void)
{
	unsigned short addr;

	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(unsigned short), (unsigned char*)&addr);
	return addr;
}

unsigned int Estimate_SendT_From_RecT(unsigned int rec_time, unsigned char rec_size)
{
	return rec_time- (0x95 + 32*(rec_size+6))*TickPerUs;
	//return rec_time- 0x95*TickPerUs - 32*(3);
}
unsigned int Estimate_SendData_Time_Length(unsigned char s_size)
{
	#define TX_DONE_EX_TIME	50 //us
	return (32*(s_size+6) + 0x95 + TX_DONE_EX_TIME);
}

unsigned char RF_Manual_Send(FunCreatPDU_Typedef pfun_creat_pdu, void* arg)
{
	unsigned char send_length;

	if((pfun_creat_pdu==NULL) || (arg==NULL))
		return 0;

	RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL);

	send_length = (*pfun_creat_pdu)(tx_buf, arg);
	RF_TxPkt(tx_buf);

	return send_length;
}
