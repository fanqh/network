/*
 * pa.c
 *
 *  Created on: 2017-5-12
 *      Author: Administrator
 */
#include "../../drivers.h"
#include "../../common.h"

#define PA_RXEN_PIN 	GPIOC_GP5
#define PA_TXEN_PIN 	GPIOD_GP2

PAMode_TypeDef pa_mode;

void PA_Control_Pin_Init(void)
{
    GPIO_SetGPIOEnable(PA_RXEN_PIN, Bit_SET);
    GPIO_ResetBit(PA_RXEN_PIN);
    GPIO_SetOutputEnable(PA_RXEN_PIN, Bit_SET);

    GPIO_SetGPIOEnable(PA_TXEN_PIN, Bit_SET);
    GPIO_ResetBit(PA_TXEN_PIN);
    GPIO_SetOutputEnable(PA_TXEN_PIN, Bit_SET);
}

void Pa_Mode_Switch(PAMode_TypeDef mode)
{
	if(pa_mode != mode)
	{
		if(mode== PA_RX_MODE)
		{
			GPIO_SetBit(PA_RXEN_PIN);
			GPIO_ResetBit(PA_TXEN_PIN);
		}
		else if(mode== PA_TX_MODE)
		{
			GPIO_SetBit(PA_TXEN_PIN);
			GPIO_ResetBit(PA_RXEN_PIN);
		}
		else
		{
			GPIO_ResetBit(PA_TXEN_PIN);
			GPIO_ResetBit(PA_RXEN_PIN);
		}
		pa_mode = mode;
	}
}


