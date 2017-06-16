/*
 * mac.h
 *
 *  Created on: 2017-6-16
 *      Author: Administrator
 */

#ifndef MAC_H_
#define MAC_H_
#include "../common.h"
#include "../drivers.h"
#include "config.h"
#include "frame.h"
typedef unsigned char (*FunCreatPDU_Typedef)(unsigned char *pbuff, void* pInfo);

unsigned short Get_MAC_Addr(void);
unsigned int Estimate_SendT_From_RecT(unsigned int rec_time, unsigned char rec_size);
unsigned char RF_Manual_Send(FunCreatPDU_Typedef pfun_creat_pdu, void* arg);

#endif /* MAC_H_ */
