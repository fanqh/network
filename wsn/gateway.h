#ifndef _GATE_WAY_H_
#define _GATE_WAY_H_

#define PALLET_DATA_LEN       64

//states of the gateway state machine
enum {
    GW_STATE_SETUP_IDLE = 0,     //0
    GW_STATE_SETUP_PALLET_REQ_WAIT,//1

    GW_STATE_SEND_GW_BCN,//2
    GW_STATE_PALLET_DATA_WAIT,//3
    GW_STATE_SEND_PALLET_ACK,//4
    GW_STATE_SUSPEND,//5
};

typedef struct {
    unsigned short pallet_addr;
    unsigned char pallet_id;
    unsigned char pallet_node_num;
    unsigned char pallet_data[PALLET_DATA_LEN];
} PalletEntry_Typedef;

typedef struct {
    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup
    unsigned int period_cnt; //the overall period count
    unsigned short mac_addr; //mac addr of the gateway
    unsigned short pallet_addr; //mac addr of current remote pallet
    unsigned char pallet_id; //assigned id for the current remote node
    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet
    unsigned char state; //current state of device
    unsigned char pallet_table_len;
} GWInfo_TypeDef;

extern void Gateway_Init(void);
extern void Gateway_MainLoop(void);
extern void Gateway_RxIrqHandler(void);
extern void Gateway_RxTimeoutHandler(void);

#endif /*_GATE_WAY_H_*/
