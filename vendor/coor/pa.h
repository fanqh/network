/*
 * pa.h
 *
 *  Created on: 2017-5-12
 *      Author: Administrator
 */


#ifndef PA_H_
#define PA_H_

//#define PA_MODE 0

typedef enum
{
	PA_SHUTDOWN_MODE=0,
	PA_TX_MODE,
	PA_RX_MODE,
}PAMode_TypeDef;

void Pa_Mode_Switch(PAMode_TypeDef mode);

#endif /* PA_H_ */
