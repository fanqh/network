#ifndef _GATE_WAY_H_
#define _GATE_WAY_H_

//states of the gateway state machine
enum {
    GW_STATE_SEND_GW_BCN = 0,
    GW_STATE_PALLET_DATA_WAIT,
    GW_STATE_SEND_PALLET_ACK,
    GW_STATE_SUSPEND,
};

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned char gw_id; //id of this gateway
    unsigned char pallet_id; //id of the pallet which is given the opportunity in this period
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char state; //current state of device
} GWInfo_TypeDef;

extern void Gateway_Init(void);
extern void Gateway_MainLoop(void);
extern void Gateway_RxIrqHandler(void);
extern void Gateway_RxTimeoutHandler(void);

#endif /*_GATE_WAY_H_*/