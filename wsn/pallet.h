#ifndef _PALLET_H_
#define _PALLET_H_

#define NODE_DATA_LEN       16

enum {
    PALLET_STATE_SETUP_IDLE = 0,
    PALLET_STATE_SETUP_NODE_REQ_WAIT,

    PALLET_STATE_SETUP_IDLE_2,
    PALLET_STATE_SETUP_GW_BCN_WAIT,
    PALLET_STATE_SETUP_BACKOFF,
    PALLET_STATE_SETUP_REQ_SEND,
    PALLET_STATE_SETUP_GW_RSP_WAIT,

    PALLET_STATE_IDLE,
    PALLET_STATE_GW_BCN_WAIT,
    PALLET_STATE_GW_ACK_WAIT,
    PALLET_STATE_SUSPEND_BEFORE_PB,
    PALLET_STATE_SEND_PB,
    PALLET_STATE_NODE_DATA_WAIT,
    PALLET_STATE_SEND_NODE_ACK,
    PALLET_STATE_SUSPEND_BEFORE_GB,
};

typedef struct {
    unsigned short node_addr;
    unsigned char node_id;
    unsigned char node_data[NODE_DATA_LEN];
} NodeEntry_Typedef;

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned short mac_addr; //mac addr of the pallet
    unsigned short node_addr; //mac addr of current remote node
    unsigned short gw_addr; //mac addr of the gateway this pallet attaches to
    unsigned char gw_id; //id of the gateway this pallet attaches to
    unsigned char pallet_id; //id of this pallet assigned by the gateway
    unsigned char node_id; //assigned id for the current remote node
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char state; //current state of device
    unsigned char retry_times; //retry times
    unsigned char node_table_len;
} PalletInfo_TypeDef;

extern void Pallet_Init(void);
extern void Pallet_MainLoop(void);
extern void Pallet_RxIrqHandler(void);
extern void Pallet_RxTimeoutHandler(void);

#endif /*_PALLET_H_*/