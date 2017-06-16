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

unsigned short Get_MAC_Addr(void);
unsigned int Estimate_SendT_From_RecT(unsigned int rec_time, unsigned char rec_size);

#endif /* MAC_H_ */
