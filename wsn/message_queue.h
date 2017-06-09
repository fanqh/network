#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include "../common.h"

//define the maximum length of message queue
#define MSG_BUF_LEN            8

enum {
    MSG_TYPE_NONE = 0,

    //gateway message type
    GW_MSG_TYPE_INVALID_DATA,  //1
    GW_MSG_TYPE_PALLET_DATA,   //2
    GW_MSG_TYPE_PALLET_DATA_TIMEOUT,//3

    GW_MSG_TYPE_SETUP_REQ,//4
    
    //pallet message type
    PALLET_MSG_TYPE_INVALID_DATA,//5
    PALLET_MSG_TYPE_GW_BCN,//6
    PALLET_MSG_TYPE_GW_ACK,//7
    PALLET_MSG_TYPE_GW_ACK_TIMEOUT,//8
    PALLET_MSG_TYPE_ED_DATA,//9
    PALLET_MSG_TYPE_ED_DATA_TIMEOUT,//10

    PN_MSG_ND_SETUP_REQ,//11
    PALLET_MSG_TYPE_SETUP_GW_BCN,//12
    PALLET_MSG_TYPE_SETUP_GW_RSP,//13
    MSG_P_SETUP_WAIT_G_TIMEOUT,//14

    //end device message type
    NODE_MSG_TYPE_INVALID_DATA,//15
    NODE_MSG_TYPE_GW_BCN,//16
    NODE_MSG_TYPE_PALLET_BCN,//17
    NODE_MSG_TYPE_PALLET_ACK,//18
    NODE_MSG_TYPE_PALLET_ACK_TIMEOUT,//19

//    NODE_MSG_TYPE_SETUP_BCN,
//    NODE_MSG_TYPE_SETUP_RSP,
//    NODE_MSG_TYPE_SETUP_RSP_TIMEOUT,
    NP_MSG_SETUP_GW_BCN,
    NP_MSG_SETUP_PLT_BCN,
    NP_MSG_SETUP_RSP,
    NP_MSG_SETUP_RSP_TIMEOUT,
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
