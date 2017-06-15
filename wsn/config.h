#ifndef _CONFIG_H_
#define _CONFIG_H_

#define TIMESLOT_LENGTH        20000 //the duration(in us) of each timeslot   GB + G/Pn
#define PALLET_NUM             6 //the number of pallets attached to each gateway
#define NODE_NUM               3 //the number of end devices attached to each pallet
#define MASTER_PERIOD		   (TIMESLOT_LENGTH*PALLET_NUM)

#define BACKOFF_UNIT           1000 //unit: us
#define BACKOFF_MAX_NUM        0x3f //maximum number of backoff unit
#define RETRY_MAX              1000 //maximum times of retry to setup with gateway request

#define PALLET_SETUP_PERIOD    (2000*PALLET_NUM*100) //us
#define GW_SETUP_BCN_NUM		10//200
#define GP_SETUP_PERIOD        (TIMESLOT_LENGTH*GW_SETUP_BCN_NUM*PALLET_NUM) //us
#define PLT_SETUP_BCN_NUM		10

//pallet setup with node
#define GW_PLT_TIME		5000 // GB + 1 communication opportunity with pallet 1ms +4ms
#define PLT_SETUP_NUM	10
#define ND_WAIT_BCN_MARGIN	5000

#define RF_TX_WAIT                      15 //in us
#define TX_ACK_WAIT                     3 //in us
#define WAIT_ACK_DONE                   (1600) //in us
#define DEV_RX_MARGIN                   20 //in us
#define RX_WAIT                         (1600) //in us
#define ACK_WAIT                        1800 //in us
#define TIMESTAMP_INVALID_THRESHOLD     3000 //in us
#define ZB_TIMESTAMP_OFFSET             341 //(6byte preamble + 0x95 pll settle time)in us
#define TX_DONE_TIMEOUT							1000

#define SETUP_SUSPNED_EARLY_WAKEUP		500

/**********************************************/
#define LAST_SETUP_REQ_MARGIN	2000	//3MS
/**********************************************/

#define TX_BUF_LEN             128
#define RX_BUF_LEN             64
#define RX_BUF_NUM             8
#define RX_BUF_INVALID_FLG     0x01

#define RF_CHANNEL             70
#define GW_ID                  0x01
#define NODE_ID                0x01

//default value
#define PALLET_ID              0x02  //由gateway分配
#define PALLET_MAC_ADDR        0xee01
#define NODE_MAC_ADDR          0xff01
#define GW_MAC_ADDR            0xdd01



#ifdef PA_MODE
	#define SW1_PIN						  GPIOA_GP0
	#define SW2_PIN						  GPIOA_GP1
	#define LED_GREEN					  GPIOC_GP4
	#define LED_RED						  GPIOB_GP1
	#define TIMING_SHOW_PIN           	  GPIOC_GP3
	#define RX_STATE_PIN				  GPIOB_GP7
	#define TX_STATE_PIN				  GPIOB_GP5
	#define ERROR_WARN_LOOP()		{while(1){WaitMs(100);GPIO_WriteBit(LED_RED, !GPIO_ReadOutputBit(LED_RED));}}
#else
	#define TIMING_SHOW_PIN            	  GPIOC_GP4
	#define GW_SETUP_TRIG_PIN         	  GPIOD_GP2
	#define PALLET_SETUP_TRIG_PIN     	  GPIOD_GP2
	#define POWER_PIN         		  	  GPIOE_GP0
	#define SHOW_DEBUG         		  	  GPIOA_GP3
	#define LED1_GREEN         		  	  GPIOC_GP3
	#define LED2_BLUE         		  	  GPIOB_GP6
	#define LED3_RED         		  	  GPIOC_GP2
	#define LED4_WHITE         		  	  GPIOB_GP4
	#define TEST_PIN         		  	  GPIOA_GP4
	#define RX_STATE_PIN				  GPIOB_GP7
	#define TX_STATE_PIN				  GPIOB_GP5
	#define ERROR_WARN_LOOP()		{while(1){WaitMs(100);GPIO_WriteBit(LED3_RED, !GPIO_ReadOutputBit(LED3_RED));}}
#endif
//device information restore address
#define FLASH_DEVICE_INFOR_ADDR   (15*4*1024)

//whether dose system enter suspend when idle
//#define SUPEND 1


#define TIME_INDICATE()			{GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));}

#define RX_INDICATE()			{GPIO_WriteBit(RX_STATE_PIN, !GPIO_ReadOutputBit(RX_STATE_PIN));}
#define TX_INDICATE()			{GPIO_WriteBit(TX_STATE_PIN, !GPIO_ReadOutputBit(TX_STATE_PIN));}

#define		SYC_WINDOW_SIZE		(2000)
#define		SYC_EXTEND_WINDOW_SIZE		5000
#endif
