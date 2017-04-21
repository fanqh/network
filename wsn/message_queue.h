#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

//define the maximum length of message queue
#define MSG_BUF_LEN            8

enum {
    MSG_TYPE_NONE = 0,

    //gateway message type
    GW_MSG_TYPE_INVALID_DATA, //1
    GW_MSG_TYPE_PALLET_DATA,  //2
    GW_MSG_TYPE_PALLET_DATA_TIMEOUT, //3
    
    //pallet message type
    PALLET_MSG_TYPE_INVALID_DATA,//4
    PALLET_MSG_TYPE_GW_BCN, //5
    PALLET_MSG_TYPE_GW_ACK, //6
    PALLET_MSG_TYPE_GW_ACK_TIMEOUT, //7
    PALLET_MSG_TYPE_ED_DATA,  //8
    PALLET_MSG_TYPE_ED_DATA_TIMEOUT,  //9

    //end device message type
    NODE_MSG_TYPE_INVALID_DATA,//10
    NODE_MSG_TYPE_GW_BCN,//11
    NODE_MSG_TYPE_PALLET_BCN,
    NODE_MSG_TYPE_PALLET_ACK,
    NODE_MSG_TYPE_PALLET_ACK_TIMEOUT,
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
extern void Message_Reset(Msg_TypeDef *msg);

#endif /*_MESSAGE_QUEUE_H_*/
