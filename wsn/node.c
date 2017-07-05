#include "../common.h"
#include "../drivers.h"
#include "config.h"
#include "frame.h"
#include "node.h"
#include "mac.h"
#include "message_queue.h"

NodeInfo_TypeDef node_info;
static MsgQueue_Typedef msg_queue;
NodeSetup_Infor_TypeDef ND_Setup_Infor;
unsigned int temp_t0;
unsigned char send_len;

volatile unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
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
    node_info.p_nd_setup_infor = &ND_Setup_Infor;
    node_info.state = ND_SETUP_IDLE;
        
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //enable irq
    IRQ_RfIrqDisable(0xffff);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT|FLD_RF_IRQ_TX);
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_Enable();
    RF_SetTxRxOff();

}

_attribute_ram_code_ void Run_NodeStatemachine(Msg_TypeDef *msg)
{
	switch(node_info.state)
	{
		case ND_CONN_IDLE:
		{
			temp_t0 = ClockTime();
	        node_info.state = ND_CONN_BCN_WAIT;
	        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
			break;
		}
		case ND_CONN_BCN_WAIT:
		{
//			if(ClockTimeExceed(temp_t0, PLT_BCN_WAIT_TIMEOUT))
//			{
//                node_info.state = ND_CONN_SUSPEND;
//                node_info.wakeup_tick = node_info.wakeup_tick + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
//                node_info.t0 = node_info.wakeup_tick;
//			}
//			else if(msg)
			{
				if(msg->type==NODE_MSG_TYPE_PALLET_BCN)
				{
					RX_INDICATE();
					node_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data)) - \
									FRAME_PLT_PB_GET_SRC_ID(msg->data)*TIMESLOT_LENGTH*TickPerUs;

					node_info.gw_sn = FRAME_PLT_PB_GET_GW_SN(msg->data);
					node_info.plt_dsn = FRAME_GET_DSN(msg->data);
					if ((node_info.gw_sn % NODE_NUM) == (node_info.node_id % NODE_NUM))
					{
						if (FRAME_PLT_PB_GET_SRC_ID(msg->data) == node_info.pallet_id)
						{
							send_len = RF_Manual_Send(Build_NodeData, (void*)&node_info);
							TX_INDICATE();
							temp_t0 = ClockTime();
							node_info.state = ND_CONN_ND_DATA_TX_DONE_WAIT;
						}
						else
						{
							node_info.state = ND_CONN_SUSPEND;
							node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id+MASTER_PERIOD)*TickPerUs;
						}
					}
					else
					{
						node_info.state = ND_CONN_SUSPEND;
						node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id + MASTER_PERIOD)*TickPerUs;
					}
				}

			}
			break;
		}
		case ND_CONN_ND_DATA_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				node_info.state = ND_CONN_PLT_ACK_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case ND_CONN_PLT_ACK_WAIT:
		{
			if(ClockTimeExceed(temp_t0, PLT_ACK_WAIT_TIMEOUT))
			{
				//TIME_INDICATE();
				node_info.state = ND_CONN_SUSPEND;
				node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id+NODE_NUM*MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
			}
			else if(msg && (msg->type==NODE_MSG_TYPE_PALLET_ACK))
			{
				RX_INDICATE();
				ACK_REC_INDICATION();
                node_info.state = ND_CONN_SUSPEND;
                node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id+NODE_NUM*MASTER_PERIOD + 100)*TickPerUs;
			}
			break;
		}
		case ND_CONN_SUSPEND:
		{
	    	RF_SetTxRxOff();

	        if(node_info.wakeup_tick - ClockTime() >1000*TickPerUs)
	        	node_info.tmp = Get_Temperature();
	        GPIO_WriteBit(POWER_PIN, 0);

	#ifdef SUPEND
	        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, node_info.wakeup_tick);
	#else
	        while((unsigned int)(ClockTime() - node_info.wakeup_tick) > BIT(30));
	#endif
	        GPIO_WriteBit(POWER_PIN, 1);
	        node_info.state = ND_CONN_IDLE;
			break;
		}
		default:
			break;
	}
}

_attribute_ram_code_ void Node_RxIrqHandler(void)
{
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);


    if ((rx_packet[13] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet)))
    {
        // Garbage packet
        MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else
    {
    	unsigned int tick = ClockTime();
    	rx_packet[8] = (unsigned char)(tick & 0xff);
    	rx_packet[9] = (unsigned char)((tick>>8) & 0xff);
    	rx_packet[10] = (unsigned char)((tick>>16) & 0xff);
    	rx_packet[11] = (unsigned char)((tick>>24) & 0xff);
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
        if (FRAME_IS_ACK_TYPE(rx_packet) && (rx_packet[15] == node_info.dsn))
        {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_PALLET_ACK);
        }
        //if it is gateway BCN frame, do sync 
        else if (FRAME_IS_GATEWAY_BEACON(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_GW_BCN);
        }
        //if it is pallet BCN frame, do sync 
        else if (FRAME_IS_PALLET_BEACON(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_PALLET_BCN);
        }
        else
        {
            MsgQueue_Push(&msg_queue, rx_packet, NODE_MSG_TYPE_INVALID_DATA);
        }
    }   
}

_attribute_ram_code_ void Node_TxDoneHandle(void)
{
	MsgQueue_Push(&msg_queue, NULL, MSG_TX_DONE);
}


_attribute_ram_code_ void Run_Node_Setup_Statemachine(Msg_TypeDef *msg)
{
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
					RX_INDICATE();
					//node_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
					node_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data));
							//- GW_PLT_TIME*TickPerUs;
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
					else
					{
						node_info.state = ND_SETUP_BACKOFF;
						node_info.wakeup_tick = ClockTime() +  \
								(((Rand() % (MASTER_PERIOD - GW_PLT_TIME - ND_WAIT_BCN_MARGIN)))/BACKOFF_UNIT)*BACKOFF_UNIT*TickPerUs;
					}

				}
				//note: first pallet beacon can't receive, the second can be received
				else if(NODE_MSG_TYPE_PALLET_BCN == msg->type)
				{
					if(FRAME_PLT_PB_GET_SRC_ID(msg->data) == node_info.pallet_id)
					{
						RX_INDICATE();
						//node_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - (ZB_TIMESTAMP_OFFSET + node_info.pallet_id*TIMESLOT_LENGTH)*TickPerUs;
						node_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data))\
										- node_info.pallet_id*TIMESLOT_LENGTH*TickPerUs;

						node_info.state = ND_CONN_SUSPEND;
						node_info.wakeup_tick = node_info.t0 + (TIMESLOT_LENGTH*node_info.pallet_id + NODE_NUM*MASTER_PERIOD)*TickPerUs;
						//pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
					}
					else
					{}
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
			if (ND_Setup_Infor.retry_times < RETRY_MAX)
			{
				send_len = RF_Manual_Send(Build_NodeSetupReq, (void*)&node_info);
				TX_INDICATE();
				temp_t0 = ClockTime();
				node_info.state = ND_SETU_REQ_TX_DONE_WAIT;
			}
			else
			{
				ERROR_WARN_LOOP();
			}
			break;
		}
		case ND_SETU_REQ_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				node_info.state = ND_SETUP_RSP_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case ND_SETUP_RSP_WAIT:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				node_info.pallet_mac = 0;
				node_info.state = ND_SETUP_SUSPEND;
				ND_Setup_Infor.retry_times++;
				node_info.wakeup_tick =  node_info.t0 + MASTER_PERIOD*TickPerUs;
				node_info.t0 = node_info.wakeup_tick;
			}
			else if (msg)
			{
				if (msg->type == NP_MSG_SETUP_RSP)
				{
					RX_INDICATE();

					//TIME_INDICATE();
					node_info.node_id = FRAME_GET_NODE_ID(msg->data);
					node_info.pallet_mac = ND_Setup_Infor.plt_mac;
					node_info.pallet_id = ND_Setup_Infor.plt_id;

					node_info.state = ND_SETUP_SUSPEND;
					node_info.is_connect = 1;
					node_info.wakeup_tick =  node_info.t0 + MASTER_PERIOD*TickPerUs;

					//GPIO_ResetBit(SHOW_DEBUG);
					CONN_INDICATION();
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
}

void Node_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;
    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);

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
