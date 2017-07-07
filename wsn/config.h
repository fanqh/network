#ifndef _CONFIG_H_
#define _CONFIG_H_

//whether dose system enter suspend when idle
#define SUPEND 1
//device information restore address
#define FLASH_DEVICE_INFOR_ADDR   (15*4*1024)

#define TX_BUF_LEN             128
#define RX_BUF_LEN             64
#define RX_BUF_NUM             8
#define RX_BUF_INVALID_FLG     0x01
#define RF_CHANNEL             70



#define TIMESLOT_LENGTH        10000 //the duration(in us) of each timeslot   GB + G/Pn
#define PALLET_NUM             6 //the number of pallets attached to each gateway
#define NODE_NUM               3 //the number of end devices attached to each pallet
#define MASTER_PERIOD		   (TIMESLOT_LENGTH*128)
#define BACKOFF_UNIT           1000 //unit: us
#define RETRY_MAX              1000 //maximum times of retry to setup with gateway request
#define GW_SETUP_BCN_NUM	   3//200
#define PLT_SETUP_BCN_NUM	   10
//pallet setup with node
#define GW_PLT_TIME				5000 // GB + 1 communication opportunity with pallet 1ms +4ms
#define ND_WAIT_BCN_MARGIN		5000


/*
 * send packet txstl + (packet length + 6 preamble)*32
 */
#define RF_TX_WAIT                      15 //in us
#define DEV_RX_MARGIN                   20 //in us
#define RX_WAIT                         (1600+2000) //in us
#define SETUP_SUSPNED_EARLY_WAKEUP		0
#define PLT_BCN_WAIT_TIMEOUT			1500   //(11+6)*32+150 = 693
#define PLT_ACK_WAIT_TIMEOUT			600


#ifdef PA_MODE
	#define SW1_PIN						  GPIOA_GP0
	#define SW2_PIN						  GPIOA_GP1
	#define LED_GREEN					  GPIOC_GP4
	#define LED_RED						  GPIOB_GP1
	#define TIMING_SHOW_PIN           	  GPIOC_GP3
	#define RX_STATE_PIN				  GPIOB_GP7
	#define TX_STATE_PIN				  GPIOB_GP5
	#define TEST_PIN					  GPIOC_GP2
	#define ERROR_WARN_LOOP()		{while(1){WaitMs(100);GPIO_WriteBit(LED_RED, !GPIO_ReadOutputBit(LED_RED));}}
#else
	#define TIMING_SHOW_PIN            	  GPIOC_GP4
	#define GW_SETUP_TRIG_PIN         	  GPIOD_GP2
	#define PALLET_SETUP_TRIG_PIN     	  GPIOD_GP2
	#define POWER_PIN         		  	  GPIOE_GP0
	#define SHOW_DEBUG         		  	  GPIOE_GP1
	#define LED1_GREEN         		  	  GPIOC_GP3
	#define LED2_BLUE         		  	  GPIOB_GP6
	#define LED3_RED         		  	  GPIOC_GP2
	#define LED4_WHITE         		  	  GPIOB_GP4
	#define TEST_PIN         		  	  GPIOA_GP0
	#define RX_STATE_PIN				  GPIOB_GP7
	#define TX_STATE_PIN				  GPIOB_GP5
	#define ERROR_WARN_LOOP()			  {while(1){WaitMs(100);GPIO_WriteBit(LED3_RED, !GPIO_ReadOutputBit(LED3_RED));}}
	#define IRQ_INDICATION()			  {GPIO_WriteBit(LED3_RED, !GPIO_ReadOutputBit(LED3_RED));}
	#define ACK_REC_INDICATION()		  {GPIO_WriteBit(LED2_BLUE, !GPIO_ReadOutputBit(LED2_BLUE));}
	#define CONN_INDICATION()			  {GPIO_SetBit(LED1_GREEN);}
	#define DISCONN_INDICATION()		  {GPIO_ResetBit(LED1_GREEN);}
#endif
#define TIME_INDICATE()			{GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));}
#define RX_INDICATE()			{GPIO_WriteBit(RX_STATE_PIN, !GPIO_ReadOutputBit(RX_STATE_PIN));}
#define TX_INDICATE()			{GPIO_WriteBit(TX_STATE_PIN, !GPIO_ReadOutputBit(TX_STATE_PIN));}
#define TOGGLE_TEST_PIN()		{GPIO_WriteBit(TEST_PIN, !GPIO_ReadOutputBit(TEST_PIN));}

#endif
