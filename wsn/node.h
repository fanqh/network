#ifndef _NODE_H_
#define _NODE_H_

#define ND_SETUP_MASK	0x100
#define ND_CONN_MASK	0x200

enum {
	ND_ERROR_STATE = 0,

	ND_SETUP_IDLE = ND_SETUP_MASK,
	ND_SETUP_BCN_WAIT,
	ND_SETUP_BACKOFF,
	ND_SETUP_REQ_SEND,
	ND_SETUP_RSP_WAIT,

	ND_CONN_IDLE = ND_CONN_MASK,
	ND_CONN_BCN_WAIT,
	ND_CONN_PLT_ACK_WAIT,
	ND_CONN_SUSPEND,

    NODE_STATE_SETUP_IDLE = 0, //0
    NODE_STATE_SETUP_BCN_WAIT, //1
    NODE_STATE_SETUP_BACKOFF,//2
    NODE_STATE_SETUP_REQ_SEND,//3
    NODE_STATE_SETUP_PALLET_RSP_WAIT,//4

    NODE_STATE_IDLE,//5
    NODE_STATE_BCN_WAIT,//6
    NODE_STATE_PALLET_ACK_WAIT,//7
    NODE_STATE_SUSPEND,//8
};

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned short mac_addr; //mac address of this node
    unsigned short pallet_mac; //id of the pallet this node attaches to
    unsigned char pallet_id; //id of the pallet this node attaches to
    unsigned char node_id; //pallet-assigned id for this node
    volatile unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char state; //current state of device
    unsigned char retry_times; //retry times
    unsigned int setup_bcn_total;
    unsigned int tmp;
} NodeInfo_TypeDef;

extern void Node_Init(void);
extern void Node_SetupLoop(void);
extern void Node_MainLoop(void);
extern void Node_RxIrqHandler(void);
extern void Node_RxTimeoutHandler(void);

#endif /*_NODE_H_*/
