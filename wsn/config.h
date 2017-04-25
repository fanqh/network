#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdarg.h>

#define TIMESLOT_LENGTH        5000 //the duration(in us) of each timeslot
#define PALLET_NUM             6 //the number of pallets attached to each gateway
#define NODE_NUM               3 //the number of end devices attached to each pallet

//just for debug
#define TIMING_SHOW_PIN                 GPIOC_GP4

#define DEBUG_SHOW_PIN                  GPIOC_GP3
#define SUSPEND_SHOW_PIN                 GPIOC_GP2


#define RF_TX_WAIT                      15 //in us
#define TX_ACK_WAIT                     3 //in us
#define WAIT_ACK_DONE                   1600 //in us
#define DEV_RX_MARGIN                   20 //in us
#define RX_WAIT                         2500 //in us
#define ACK_WAIT                        1800 //in us
#define TIMESTAMP_INVALID_THRESHOLD     6000 //in us
#define ZB_TIMESTAMP_OFFSET             341 //(6byte preamble + 0x95 pll settle time)in us

#define TX_BUF_LEN                      128
#define RX_BUF_LEN                      64
#define RX_BUF_NUM                      4
#define RX_BUF_INVALID_FLG              0x01

#define RF_CHANNEL             60
#define GW_ID                  0x01
#define PALLET_ID              0x01
#define NODE_ID                0x02

//#define DEBUG
extern int my_printf(const char *format, ...);
#ifdef DEBUG
	#define LOG  my_printf
#else
	#define LOG
#endif

#endif
