#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "node.h"

NodeInfo_TypeDef node_info;
static MsgQueue_Typedef msg_queue;

static volatile unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_ptr = 0;


typedef struct
{
	unsigned short node_mac;
	unsigned char pallet_id;
	unsigned char rst;
}device_infor_t;
device_infor_t device_infor;


extern unsigned int Get_Temperature(void);
void Node_Init(void)
{


	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(device_infor_t), (unsigned char*)&device_infor);
	if((device_infor.pallet_id == 0xff) || (device_infor.node_mac == 0xffff))
	{
		device_infor.pallet_id = 0x01;
		device_infor.node_mac = 0xff01;
	}

    memset(&node_info, 0, sizeof(NodeInfo_TypeDef));
    node_info.mac_addr = device_infor.node_mac;
    node_info.dsn = node_info.mac_addr & 0xff;
    node_info.pallet_id = device_infor.pallet_id; //应该由gate分配
        
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
   
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
}


//unsigned char aa[8][32], a,j;
//unsigned int ttt, dd;

_attribute_ram_code_ void Run_NodeStatemachine(Msg_TypeDef *msg)
{
    unsigned int now;
    unsigned int timestamp;

    if (NODE_STATE_IDLE == node_info.state) {
        node_info.state = NODE_STATE_BCN_WAIT;
        RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
        //GPIO_WriteBit(DEBUG_PIN, !GPIO_ReadOutputBit(DEBUG_PIN));
    }
    else if (NODE_STATE_BCN_WAIT == node_info.state)
    {
        if (msg)
        {
            if (NODE_MSG_TYPE_GW_BCN == msg->type)
            {
                now = ClockTime();
                timestamp = FRAME_GET_TIMESTAMP(msg->data);
                //check validity of timestamp
                if ((unsigned int)(now - timestamp) > 6*1000*TickPerUs) {
                   // msg->type == MSG_TYPE_NONE;
                    return;
                }
                node_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
                node_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);

                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (NODE_MSG_TYPE_PALLET_BCN == msg->type)
            {
                now = ClockTime();
                timestamp = FRAME_GET_TIMESTAMP(msg->data);
                //check validity of timestamp
                if ((unsigned int)(now - timestamp) > 6*1000*TickPerUs) {
                   // msg->type == MSG_TYPE_NONE;
                    return;
                }
                unsigned short tmp_pallet_id = FRAME_GET_SRC_ADDR(msg->data);
                node_info.t0 = timestamp - (ZB_TIMESTAMP_OFFSET + tmp_pallet_id*TIMESLOT_LENGTH)*TickPerUs;  //gateway beacon 时间
                node_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);
                // if the PB is originated from the pallet this end device attaches to, determine
                // whether it is this end device's opportunity
                if ((node_info.period_cnt % NODE_NUM) == (node_info.node_id % NODE_NUM))
                {
                    if (tmp_pallet_id == node_info.pallet_id)
                    {
                        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                        RF_SetTxRxOff();
                        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); 
                        RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
                        Build_NodeData(tx_buf, &node_info);
                        //update state
                        node_info.state = NODE_STATE_PALLET_ACK_WAIT;
                        return;
                    }
                }
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+NODE_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_PALLET_ACK_WAIT == node_info.state) {
        if (msg) {
            //GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
            //node_info.state = NODE_STATE_SUSPEND;
            //node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+PALLET_NUM) - DEV_RX_MARGIN)*TickPerUs;

#if 1
            if (msg->type == NODE_MSG_TYPE_PALLET_ACK) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+NODE_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == NODE_MSG_TYPE_PALLET_ACK_TIMEOUT) {
                //GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+NODE_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == NODE_MSG_TYPE_INVALID_DATA) {
                //GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+NODE_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            else
            {
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+NODE_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
#endif
            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_SUSPEND == node_info.state) {
        //turn off receiver and go to suspend
        //RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
    	RF_SetTxRxOff();

        if(node_info.wakeup_tick - ClockTime() >1000*TickPerUs)
        	node_info.tmp = Get_Temperature();
#ifdef SUPEND
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick);
#else
        while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
#endif

        node_info.state = NODE_STATE_IDLE;
    }
}

void Node_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;
    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    //run state machine
    Run_NodeStatemachine(pMsg);
    //Message_Reset(pMsg);

    //collect sensor data

}

_attribute_ram_code_ void Node_RxIrqHandler(void)
{
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);


    if ((rx_packet[13] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet))) {
        // Garbage packet
        MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else {
//    	aa[j][31] =ttt++;
//    	memcpy(aa[j++], rx_packet, 31);
//    	if(j>=8)
//    		j = 0;

        //if it is pallet setup beacon frame, perform a random backoff and then require to associate
        if (FRAME_IS_SETUP_PALLET_BEACON(rx_packet))
        {

            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_SETUP_BCN);
        }
        //if it is pallet setup response frame, check whether the dst addr matches the local addr
        if (FRAME_IS_SETUP_PALLET_RSP(rx_packet))
        {
            unsigned short dst_addr = FRAME_GET_DST_ADDR(rx_packet);
            if (dst_addr == node_info.mac_addr)
            {
                MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_SETUP_RSP);
            }
            else
            {
                MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_INVALID_DATA);
            }
        }
        //if it is pallet ACK frame, check it and then go to suspend immediately
        if (FRAME_IS_ACK_TYPE(rx_packet) && (rx_packet[15] == node_info.dsn)) {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_PALLET_ACK);
        }
        //if it is gateway BCN frame, do sync 
        else if (FRAME_IS_GATEWAY_BEACON(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_GW_BCN);
        }
        //if it is pallet BCN frame, do sync 
        else if (FRAME_IS_PALLET_BEACON(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_PALLET_BCN);
        }
        else {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_INVALID_DATA);
        }
    }   
}

_attribute_ram_code_ void Node_RxTimeoutHandler(void)
{
    if (NODE_STATE_PALLET_ACK_WAIT == node_info.state)
    {
        MsgQueue_Push(&msg_queue, NULL, NODE_MSG_TYPE_PALLET_ACK_TIMEOUT);
    }
    else if (NODE_STATE_SETUP_PALLET_RSP_WAIT == node_info.state)
    {
        MsgQueue_Push(&msg_queue, NULL, NODE_MSG_TYPE_SETUP_RSP_TIMEOUT);
    }
    else
    {
    	//todo need add some code to tell application
    }
}

_attribute_ram_code_ void Run_Node_Setup_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;

    if (NODE_STATE_SETUP_IDLE == node_info.state) {
    	RF_SetTxRxOff();
        node_info.state = NODE_STATE_SETUP_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on waiting for mesh setup beacon
    }
    else if (NODE_STATE_SETUP_BCN_WAIT == node_info.state) {
        if (msg) {
            if (NODE_MSG_TYPE_SETUP_BCN == msg->type) {
                now = ClockTime();
                node_info.state = NODE_STATE_SETUP_BACKOFF;
                node_info.retry_times = 0;
                node_info.wakeup_tick = now + ((Rand()+node_info.mac_addr) & BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs;
                node_info.pallet_mac = FRAME_GET_SRC_ADDR(msg->data);
                //TODO  pallet ID 应该由gateway 分配，现在是固定值，因为此时还没有和gateway关联
                //node_info.pallet_id = PALLET_ID;
            }
            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_SETUP_BACKOFF == node_info.state) {
        //turn off receiver and go to suspend
        RF_SetTxRxOff();
        // while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //turn off RX mode

        WaitMs(5);
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick);
        node_info.state = NODE_STATE_SETUP_REQ_SEND;
    }
    else if (NODE_STATE_SETUP_REQ_SEND == node_info.state) {
        if (node_info.retry_times < RETRY_MAX) {
            now = ClockTime();
            RF_StartStxToRx(tx_buf, now + RF_TX_WAIT*TickPerUs, RX_WAIT);
            Build_NodeSetupReq(tx_buf, &node_info);
            //GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
            //update state
            node_info.state = NODE_STATE_SETUP_PALLET_RSP_WAIT;
        }
        else {
            //blink led and stall
            while (1) {
                GPIO_SetBit(GPIOC_GP2);
                WaitMs(20);
                GPIO_ResetBit(GPIOC_GP2);
                WaitMs(80);
            }
        }
    }
    else if (NODE_STATE_SETUP_PALLET_RSP_WAIT == node_info.state) {
        if (msg) {
            if (msg->type == NODE_MSG_TYPE_SETUP_RSP) {
               // GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.node_id = FRAME_GET_NODE_ID(msg->data);
                //node_info.pallet_id = PALLET_ID;
                node_info.state = NODE_STATE_IDLE;
            }
            else {
               // GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                now = ClockTime();
                node_info.state = NODE_STATE_SETUP_BACKOFF;
                node_info.wakeup_tick = now + ((Rand()+node_info.mac_addr) & BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs;
                node_info.retry_times++;
            }

            Message_Reset(msg);
        }
    }
}

void Node_SetupLoop(void)
{
    Msg_TypeDef* pMsg = NULL;

    while (NODE_STATE_IDLE != node_info.state)
    {
        //pop a message from the message queue
        pMsg = MsgQueue_Pop(&msg_queue);
        //run state machine
        Run_Node_Setup_Statemachine(pMsg);
    }
}
