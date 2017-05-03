#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

//define the maximum length of message queue
#define MSG_BUF_LEN            8

enum {
    MSG_TYPE_NONE = 0,

    //gateway message type
    GW_MSG_TYPE_INVALID_DATA,
    GW_MSG_TYPE_PALLET_DATA,
    GW_MSG_TYPE_PALLET_DATA_TIMEOUT,

    GW_MSG_TYPE_SETUP_REQ,
    
    //pallet message type
    PALLET_MSG_TYPE_INVALID_DATA,
    PALLET_MSG_TYPE_GW_BCN,
    PALLET_MSG_TYPE_GW_ACK,
    PALLET_MSG_TYPE_GW_ACK_TIMEOUT,
    PALLET_MSG_TYPE_ED_DATA,
    PALLET_MSG_TYPE_ED_DATA_TIMEOUT,

    PALLET_MSG_TYPE_SETUP_REQ,

    PALLET_MSG_TYPE_SETUP_GW_BCN,
    PALLET_MSG_TYPE_SETUP_GW_RSP,
    PALLET_MSG_TYPE_SETUP_GW_RSP_TIMEOUT,

    //end device message type
    NODE_MSG_TYPE_INVALID_DATA,
    NODE_MSG_TYPE_GW_BCN,
    NODE_MSG_TYPE_PALLET_BCN,
    NODE_MSG_TYPE_PALLET_ACK,
    NODE_MSG_TYPE_PALLET_ACK_TIMEOUT,

    NODE_MSG_TYPE_SETUP_BCN,
    NODE_MSG_TYPE_SETUP_RSP,
    NODE_MSG_TYPE_SETUP_RSP_TIMEOUT,
};

typedef struct {
    unsigned char *data; //content of msg
    unsigned char type; //type of msg 
    unsigned char reserved[3];
    //unsigned char buff[32];
} Msg_TypeDef;

typedef struct {
    Msg_TypeDef msg[MSG_BUF_LEN]; //buffer storing msg
    //unsigned char cnt; //current num of msg
    unsigned char read; //ptr of the msg should be read next in the queue
    unsigned char write;
} MsgQueue_Typedef;

extern int MsgQueue_Push(MsgQueue_Typedef *p, unsigned char *data, unsigned char type);
extern Msg_TypeDef *MsgQueue_Pop(MsgQueue_Typedef *p);
_attribute_ram_code_ int MsgQueue_Clean(MsgQueue_Typedef *p);
_attribute_ram_code_ void Message_Reset(Msg_TypeDef *msg);

#endif /*_MESSAGE_QUEUE_H_*/
