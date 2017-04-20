#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "node.h"

static NodeInfo_TypeDef node_info;
static MsgQueue_Typedef msg_queue;

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;

void Node_Init(void)
{
    memset(&node_info, 0, sizeof(NodeInfo_TypeDef));
    node_info.pallet_id = PALLET_ID;
    node_info.node_id = NODE_ID;
    node_info.dsn = (NODE_ID + PALLET_ID) * 10;
        
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
   
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
}

_attribute_session_(".ram_code") void Run_NodeStatemachine(Msg_TypeDef *msg)
{
    unsigned int now;
    unsigned int timestamp;

    if (NODE_STATE_IDLE == node_info.state) {
        node_info.state = NODE_STATE_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
    }
    else if (NODE_STATE_BCN_WAIT == node_info.state) {
        if (msg) {
            if (NODE_MSG_TYPE_GW_BCN == msg->type) {
                now = ClockTime();
                timestamp = FRAME_GET_TIMESTAMP(msg->data);
                //check validity of timestamp
                if ((unsigned int)(now - timestamp) > 6*1000*TickPerUs) {
                    msg->type == MSG_TYPE_NONE;
                    return;
                }
                node_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
                node_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);

                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (NODE_MSG_TYPE_PALLET_BCN == msg->type) {
                now = ClockTime();
                timestamp = FRAME_GET_TIMESTAMP(msg->data);
                //check validity of timestamp
                if ((unsigned int)(now - timestamp) > 6*1000*TickPerUs) {
                    msg->type == MSG_TYPE_NONE;
                    return;
                }
                unsigned char tmp_pallet_id = FRAME_GET_PALLET_ID(msg->data);
                node_info.t0 = timestamp - (ZB_TIMESTAMP_OFFSET + tmp_pallet_id*TIMESLOT_LENGTH)*TickPerUs;
                node_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);
                // if the PB is originated from the pallet this end device attaches to, determine
                // whether it is this end device's opportunity
                if ((node_info.period_cnt % NODE_NUM) == (node_info.node_id % NODE_NUM)) {
                    if (tmp_pallet_id == node_info.pallet_id) {
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
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+PALLET_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_PALLET_ACK_WAIT == node_info.state) {
        if (msg) {
            if (msg->type == NODE_MSG_TYPE_PALLET_ACK) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+PALLET_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == NODE_MSG_TYPE_PALLET_ACK_TIMEOUT) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+PALLET_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == NODE_MSG_TYPE_INVALID_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                node_info.state = NODE_STATE_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*(node_info.pallet_id+PALLET_NUM) - DEV_RX_MARGIN)*TickPerUs;
            }
            Message_Reset(msg);
        }
    }
    else if (NODE_STATE_SUSPEND == node_info.state) {
        //turn off receiver and go to suspend
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
        // while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick);
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

    //collect sensor data

}

_attribute_session_(".ram_code") void Node_RxIrqHandler(void)
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

_attribute_session_(".ram_code") void Node_RxTimeoutHandler(void)
{
    if (NODE_STATE_PALLET_ACK_WAIT == node_info.state) {
        MsgQueue_Push(&msg_queue, NULL, NODE_MSG_TYPE_PALLET_ACK_TIMEOUT);
    }
    else {

    }
}
