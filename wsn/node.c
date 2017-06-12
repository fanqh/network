#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "node.h"

NodeInfo_TypeDef node_info;
static MsgQueue_Typedef msg_queue;
NodeSetup_Infor_TypeDef ND_Setup_Infor;
unsigned int temp_t0;
unsigned char bug;

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

    memset(&node_info, 0, sizeof(NodeInfo_TypeDef));
    node_info.mac_addr = device_infor.node_mac;
    node_info.dsn = node_info.mac_addr & 0xff;
    //node_info.pallet_id = device_infor.pallet_id; //应该由gate分配
    node_info.p_nd_setup_infor = &ND_Setup_Infor;
    node_info.state = ND_SETUP_IDLE;
        
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
   
    //enable irq

    IRQ_RfIrqDisable(0xffff);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_Enable();

}


//unsigned char aa[8][32], a,j;
//unsigned int ttt, dd;

_attribute_ram_code_ void Run_NodeStatemachine(Msg_TypeDef *msg)
{
#if 0
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
#endif
}

void Node_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;
    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    ev_process_timer();

    if(node_info.state & ND_SETUP_MASK)
    {
    	Run_Node_Setup_Statemachine(pMsg);
    }
    else if(node_info.state & ND_CONN_MASK)
    {
    	Run_NodeStatemachine(pMsg);
    }
    else
    {
    	ERROR_WARN_LOOP();
    }

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
        //if it is pallet setup beacon frame, perform a random backoff and then require to associate
        if (FRAME_IS_SETUP_PALLET_BEACON(rx_packet))
        {

            MsgQueue_Push(&msg_queue, rx_packet, NP_MSG_SETUP_PLT_BCN);
        }
        //if it is pallet setup response frame, check whether the dst addr matches the local addr
        if (FRAME_IS_SETUP_PALLET_RSP(rx_packet))
        {

        	unsigned short dst_addr = FRAME_PLT_SETUP_RSP_DEST_ADDR(rx_packet);
        	unsigned short src_addr = FRAME_PLT_SETUP_RSP_SRC_ADDR(rx_packet);

            if ((src_addr == ND_Setup_Infor.plt_mac) &&(dst_addr == node_info.mac_addr))
            {
                MsgQueue_Push(&msg_queue, rx_packet, NP_MSG_SETUP_RSP);
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
//    if (NODE_STATE_PALLET_ACK_WAIT == node_info.state)
//    {
//        MsgQueue_Push(&msg_queue, NULL, NODE_MSG_TYPE_PALLET_ACK_TIMEOUT);
//    }
//    else if (NODE_STATE_SETUP_PALLET_RSP_WAIT == node_info.state)
//    {
//        MsgQueue_Push(&msg_queue, NULL, NP_MSG_SETUP_RSP_TIMEOUT);
//    }
//    else
//    {
//    	//todo need add some code to tell application
//    }
}

unsigned char Wait_Tx_Done(unsigned int timeout)//unit : us
{
	unsigned int t;

	t = ClockTime();
	while(!RF_TxFinish())
	{
		if(ClockTimeExceed(t, timeout))
			return FAILURE;
	}
	//RF_TxFinishClearFlag();
	return SUCCESS;
}
_attribute_ram_code_ void Run_Node_Setup_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;

    switch(node_info.state)
    {
    case ND_SETUP_IDLE:
    {
    	RF_SetTxRxOff();
        node_info.state = ND_SETUP_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on waiting for mesh setup beacon
        break;
    }
    case ND_SETUP_BCN_WAIT:
    {
        if (msg)
        {
            if (NP_MSG_SETUP_GW_BCN == msg->type)
            {

            }
            else if(NP_MSG_SETUP_PLT_BCN == msg->type)
            {
            	GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));

                node_info.state = ND_SETUP_BACKOFF;
                node_info.retry_times = 0;
                node_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                node_info.setup_bcn_total = 200;
                ND_Setup_Infor.plt_mac = FRAME_PLT_SETUP_BCN_GET_SRC_MAC(msg->data);
                ND_Setup_Infor.plt_id = FRAME_PLT_SETU_BCN_GET_PLT_ID(msg->data);
                if(node_info.is_connect==1)
                {
                	if((ND_Setup_Infor.plt_mac==node_info.pallet_mac)&&(ND_Setup_Infor.plt_id==node_info.pallet_id))
                	{
                		node_info.state = ND_SETUP_SUSPEND;
                		node_info.wakeup_tick  = node_info.t0 + MASTER_PERIOD*TickPerUs;
                		break;
                	}
                }
                node_info.wakeup_tick = ClockTime() +  (((Rand() % (MASTER_PERIOD - GW_PLT_TIME - ND_WAIT_BCN_MARGIN)))/BACKOFF_UNIT)*BACKOFF_UNIT*TickPerUs;
            }
            else
            {

            }
            Message_Reset(msg);
        }
        break;
    }
    case ND_SETUP_BACKOFF:
    {
		RF_SetTxRxOff();
		GPIO_WriteBit(POWER_PIN, 0);
#if SUPEND
		PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
#else
		while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
#endif
		GPIO_WriteBit(POWER_PIN, 1);
        node_info.state = ND_SETUP_REQ_SEND;
    	break;
    }
    case ND_SETUP_REQ_SEND:
    {
        if (node_info.retry_times < RETRY_MAX)
        {
#if 0
            RF_SetTxRxOff();
            RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //switch to auto mode
            RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, RX_WAIT);
            Build_NodeSetupReq(tx_buf, &node_info);
            node_info.state = ND_SETUP_RSP_WAIT;
            temp_t0 = ClockTime();
            GPIO_SetBit(SHOW_DEBUG);
#else
	        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
	        Build_NodeSetupReq(tx_buf, &node_info);

	        TIME_INDICATE();
	        RF_TxPkt(tx_buf);

	        if(Wait_Tx_Done(TX_DONE_TIMEOUT) == FAILURE)
	        {
	        	ERROR_WARN_LOOP();
	        }
	        TIME_INDICATE();

        	temp_t0 = ClockTime();
        	GPIO_SetBit(SHOW_DEBUG);
        	RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on waiting for mesh setup beacon

            node_info.state = ND_SETUP_RSP_WAIT;
#endif
        }
        else
        {
        	ERROR_WARN_LOOP();
        }
    	break;
    }
    case ND_SETUP_RSP_WAIT:
    {
    	if(ClockTimeExceed(temp_t0, 3000))
		{
			TIME_INDICATE();
        	node_info.pallet_mac = 0;
            node_info.state = ND_SETUP_SUSPEND;
            node_info.retry_times++;
            node_info.wakeup_tick =  node_info.t0 + MASTER_PERIOD*TickPerUs;
            GPIO_ResetBit(SHOW_DEBUG);
		}
		else if (msg)
        {
            if (msg->type == NP_MSG_SETUP_RSP)
            {
            	TIME_INDICATE();
                node_info.node_id = FRAME_GET_NODE_ID(msg->data);
                node_info.pallet_mac = ND_Setup_Infor.plt_mac;
                node_info.pallet_id = ND_Setup_Infor.plt_id;
                GPIO_SetBit(LED1_GREEN);
                node_info.state = ND_SETUP_SUSPEND;
                node_info.is_connect = 1;
                node_info.wakeup_tick =  node_info.t0 + MASTER_PERIOD*TickPerUs;
                Message_Reset(msg);
            }
        }
    	break;
    }
    case ND_SETUP_SUSPEND:
    {
		RF_SetTxRxOff();
		GPIO_WriteBit(POWER_PIN, 0);
#if SUPEND
		PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
#else
		while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
#endif
		GPIO_WriteBit(POWER_PIN, 1);
        node_info.state = ND_SETUP_IDLE;
    	break;
    }

    default:
    	break;
    }
#if 0
    if (ND_SETUP_IDLE == node_info.state)
    {
    	RF_SetTxRxOff();
        node_info.state = ND_SETUP_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on waiting for mesh setup beacon
    }
    else if (ND_SETUP_BCN_WAIT == node_info.state)
    {
        if (msg)
        {
            if (NP_MSG_SETUP_GW_BCN == msg->type)
            {

            }
            else if(NP_MSG_SETUP_PLT_BCN == msg->type)
            {
                node_info.state = NODE_STATE_SETUP_BACKOFF;
                node_info.retry_times = 0;

                node_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                node_info.setup_bcn_total = 200;
                node_info.pallet_mac = FRAME_GET_SRC_ADDR(msg->data);

                node_info.wakeup_tick = ClockTime() +  (((Rand() % (MASTER_PERIOD - GW_PLT_TIME - ND_WAIT_BCN_MARGIN)))/BACKOFF_UNIT)*BACKOFF_UNIT*TickPerUs;
            }
            else
            {

            }
            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_SETUP_BACKOFF == node_info.state)
    {
        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //turn off RX mode
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick);

        node_info.state = NODE_STATE_SETUP_REQ_SEND;
    }
    else if (NODE_STATE_SETUP_REQ_SEND == node_info.state)
    {
        if (node_info.retry_times < RETRY_MAX)
        {
            now = ClockTime();
            RF_StartStxToRx(tx_buf, now + RF_TX_WAIT*TickPerUs, RX_WAIT);
            Build_NodeSetupReq(tx_buf, &node_info);
            TIME_INDICATE();

            node_info.state = NODE_STATE_SETUP_PALLET_RSP_WAIT;
        }
        else
        {
        	ERROR_WARN_LOOP();
        }
    }
    else if (NODE_STATE_SETUP_PALLET_RSP_WAIT == node_info.state)
    {
        if (msg)
        {
            if (msg->type == NODE_MSG_TYPE_SETUP_RSP)
            {
                node_info.node_id = FRAME_GET_NODE_ID(msg->data);
                node_info.state = NODE_STATE_IDLE;
            }
            else
            {
                node_info.state = ND_SETUP_IDLE;
                node_info.retry_times++;
            }
            TIME_INDICATE();
            Message_Reset(msg);
        }
    }
#endif
}
//
//void Node_SetupLoop(void)
//{
//    Msg_TypeDef* pMsg = NULL;
//
//    while (NODE_STATE_IDLE != node_info.state)
//    {
//        //pop a message from the message queue
//        pMsg = MsgQueue_Pop(&msg_queue);
//        //run state machine
//        Run_Node_Setup_Statemachine(pMsg);
//    }
//}
