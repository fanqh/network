#ifndef _PALLET_H_
#define _PALLET_H_

enum {
    PALLET_STATE_IDLE = 0,
    PALLET_STATE_GW_BCN_WAIT,
    PALLET_STATE_GW_ACK_WAIT,
    PALLET_STATE_SUSPEND_BEFORE_PB,
    PALLET_STATE_SEND_PB,
    PALLET_STATE_NODE_DATA_WAIT,
    PALLET_STATE_SEND_NODE_ACK,
    PALLET_STATE_SUSPEND_BEFORE_GB,
};

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned char gw_id; //id of the gateway this pallet attaches to
    unsigned char pallet_id; //id of this pallet
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char state; //current state of device
} PalletInfo_TypeDef;

extern void Pallet_Init(void);
extern void Pallet_MainLoop(void);
extern void Pallet_RxIrqHandler(void);
extern void Pallet_RxTimeoutHandler(void);

#endif /*_PALLET_H_*/