#ifndef _GATE_WAY_H_
#define _GATE_WAY_H_

#include "mac_data.h"

#define CONN_PLT_NUM_MAX       32

//states of the gateway state machine
#define GW_SETUP_MASK		0X100
#define GW_ASSOCIATE_MASK	0X200
typedef enum {
	GW_STATE_RF_OFF,
    GW_SETUP_IDLE = GW_SETUP_MASK,
    GW_SETUP_SEND_GB,
    GW_SETUP_GB_TX_DONE_WAIT,
    GW_SETUP_PLT_REQ_WAIT,
    GW_SETUP_RSP_TX_DONE,
    GW_SETUP_SUSPEND,

    GW_CONN_SEND_GW_BCN = GW_ASSOCIATE_MASK,
    GW_CONN_GB_TX_DONE,
    GW_CONN_PLT_DATA_WAIT,
    GW_CONN_SEND_PLT_DATA_ACK,
    GW_CONN_DATA_ACK_TX_DONE,
    GW_CONN_SUSPEND,
}GW_StateTypedef;

#define IS_GW_WITHIN_SETUP_STATE(state)		(state & GW_SETUP_MASK)
#define IS_GW_WITHIN_ASSOCIATE_STATE(state)	(state & GW_ASSOCIATE_MASK)


typedef struct {
    unsigned short pallet_addr;
    unsigned char pallet_id;
    unsigned char pallet_node_num;
    unsigned char pallet_data[32];
} PalletEntry_Typedef;

typedef struct{
	unsigned char plt_id;
	unsigned char setup_num;
	unsigned short plt_addr;
}GatewaySetupInfor_Typedef;



typedef struct {
	GW_StateTypedef state; //current state of device
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned char gw_id;
    unsigned short mac_addr; //mac addr of the gateway
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char setup_dsn;

    GatewaySetupInfor_Typedef *pSetup_info;
    Conn_List_Typedef *pConn_list;
} GWInfo_TypeDef;

extern void Gateway_Init(void);
extern void Gateway_MainLoop(void);
extern void Gateway_RxIrqHandler(void);
extern void Gateway_RxTimeoutHandler(void);
extern void Gateway_TxDoneHandle(void);
void Gateway_SetupLoop(void);

#endif /*_GATE_WAY_H_*/
