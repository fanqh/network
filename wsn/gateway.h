#ifndef _GATE_WAY_H_
#define _GATE_WAY_H_

#define CONN_PLT_NUM_MAX       32

//states of the gateway state machine
#define GW_SETUP_MASK		0X10
#define GW_ASSOCIATE_MASK	0X20
enum {
	GW_STATE_RF_OFF,
    GW_STATE_SETUP_IDLE = GW_SETUP_MASK,
    GW_STATE_SETUP_SEND_GB,
    GW_STATE_SETUP_PALLET_REQ_WAIT,

    GW_STATE_SEND_GW_BCN = GW_ASSOCIATE_MASK,
    GW_STATE_PALLET_DATA_WAIT,
    GW_STATE_SEND_PALLET_ACK,
    GW_STATE_SUSPEND,
};

#define IS_GW_WITHIN_SETUP_STATE(state)		(state & GW_SETUP_MASK)
#define IS_GW_WITHIN_ASSOCIATE_STATE(state)	(state & GW_ASSOCIATE_MASK)


typedef struct {
    unsigned short pallet_addr;
    unsigned char pallet_id;
    unsigned char pallet_node_num;
    unsigned char pallet_data[32];
} PalletEntry_Typedef;

typedef struct {
	unsigned char state; //current state of device
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned short mac_addr; //mac addr of the gateway
    unsigned short pallet_addr; //mac addr of current remote pallet
    unsigned char pallet_id; //assigned id for the current remote node
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char pallet_table_len;
} GWInfo_TypeDef;

extern void Gateway_Init(void);
extern void Gateway_MainLoop(void);
extern void Gateway_RxIrqHandler(void);
extern void Gateway_RxTimeoutHandler(void);
void Gateway_SetupLoop(void);

#endif /*_GATE_WAY_H_*/
