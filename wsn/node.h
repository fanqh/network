#ifndef _NODE_H_
#define _NODE_H_

enum {
    NODE_STATE_IDLE = 0,
    NODE_STATE_BCN_WAIT,
    NODE_STATE_PALLET_ACK_WAIT,
    NODE_STATE_SUSPEND,
};

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned char pallet_id; //id of the pallet this end device attaches to
    unsigned char node_id; //id of this end device
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char state; //current state of device
} NodeInfo_TypeDef;

extern void Node_Init(void);
extern void Node_MainLoop(void);
extern void Node_RxIrqHandler(void);
extern void Node_RxTimeoutHandler(void);

#endif /*_NODE_H_*/