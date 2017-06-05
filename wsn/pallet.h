#ifndef _PALLET_H_
#define _PALLET_H_

#define NODE_DATA_LEN       16

#define PN_SETUP_STATE_MASK  	0X100
#define GP_SETUP_STATE_MASK  	0X200
#define GPN_CONSIGN_STATE_MASK	0X400
#define GP_KEEP_SYC_MASK		0X800
typedef enum {
	//setup with node
    S_PN_SETUP_IDLE = PN_SETUP_STATE_MASK,   //0x100
    PALLET_STATE_GW_BCN_WAIT0 ,
    PALLET_STATE_SETUP_BACKOFF0,
    PALLET_STATE_SETUP_SEND_BCN,
    PALLET_STATE_SETUP_NODE_REQ_WAIT,
    PN_SETUP_SUSPEND,

    //setup with gateway
    PALLET_STATE_OFF = GP_SETUP_STATE_MASK,  //0x200
    GP_SETUP_IDLE,
    GP_SETUP_BCN_WAIT,
    GP_SETUP_BACKOFF,
    GP_SETUP_REQ_SEND,
    GP_SETUP_GW_RSP_WAIT,

    //keep listen gateway and suspend alternate
    GP_SYC_LISTEN_GB=GP_KEEP_SYC_MASK,  //0x400
    GP_SYC_SUSPNED,
    GP_SYC_ACK_WAIT,

    //consign state
    PALLET_STATE_IDLE = GPN_CONSIGN_STATE_MASK, //0x800
    PALLET_STATE_GW_BCN_WAIT,
    PALLET_STATE_GW_ACK_WAIT,
    PALLET_STATE_SUSPEND_BEFORE_PB,
    PALLET_STATE_SEND_PB,
    PALLET_STATE_NODE_DATA_WAIT,
    PALLET_STATE_SEND_NODE_ACK,
    PALLET_STATE_SUSPEND_BEFORE_GB,
}PALLET_StateTypeDef;

#define IS_STATE_PALLET_SETUP_WITH_NODE(state) 			(state & PN_SETUP_STATE_MASK)
#define IS_STATE_PALLET_SETUP_WITH_GATEWAY(state) 		(state & GP_SETUP_STATE_MASK)
#define IS_STATE_PALLET_KEEP_SYC_WITH_GW(state)			(state & GP_KEEP_SYC_MASK)
#define IS_STATE_PALLET_CONSIGN(state) 					(state & GPN_CONSIGN_STATE_MASK)

typedef struct {
    unsigned short node_addr;
    unsigned char node_id;
    unsigned char node_data[NODE_DATA_LEN];
} NodeEntry_Typedef;


typedef struct {
	PALLET_StateTypeDef state; //current state of device

    unsigned int t0; //the moment starting to send the beacon 
    unsigned int wakeup_tick; //the moment wakeup

    //setup with node
    unsigned char gsn;

    unsigned int period_cnt; //the overall period count
    unsigned short mac_addr; //mac addr of the pallet


    unsigned short node_addr; //mac addr of current remote node
    unsigned short gw_addr; //mac addr of the gateway this pallet attaches to
    unsigned char gw_id; //id of the gateway this pallet attaches to
    unsigned char pallet_id; //id of this pallet assigned by the gateway
    unsigned char node_id; //assigned id for the current remote node

    unsigned char dsn; //sequence number of outgoing packet
    unsigned char ack_dsn; //sequence number of incoming packet

    unsigned char gw_sn;
    unsigned char gw_setup_bcn_total;
    unsigned char retry_times; //retry times
    unsigned char node_table_len;
    unsigned char is_associate;
	//NodeDataWaitSend_Typdedef *pNodeData;
} PalletInfo_TypeDef;

extern void Pallet_Init(void);
extern void Pallet_MainLoop(void);
extern void Pallet_RxIrqHandler(void);
extern void Pallet_RxTimeoutHandler(void);
void Pallet_SetupLoop(void);
void Pallet_SetupLoop2(void);

#endif /*_PALLET_H_*/
