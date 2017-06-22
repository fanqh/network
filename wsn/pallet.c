#include "../common.h"
#include "../drivers.h"
#include "config.h"
#include "frame.h"
#include "message_queue.h"
#include "pallet.h"
#include "../debug/debug_queue.h"
#include "mac.h"

#define NODE_TABLE_MAX_LEN   6

volatile unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
volatile unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_ptr = 0;

volatile unsigned char PalletSetupTrig = 0;
static unsigned int temp_t0;
static unsigned char send_len;

PalletInfo_TypeDef pallet_info;
PalletSetup_Infor_TypeDef plt_setup_infor;
static MsgQueue_Typedef msg_queue;
static NodeEntry_Typedef node_table[NODE_TABLE_MAX_LEN];
/**************temporary data struct*******************/
typedef struct
{
	unsigned short pallet_mac;
	unsigned char pallet_id;
	unsigned char rst;
}device_infor_t;

NodeDataWaitSend_Typdedef node_data[3];
device_infor_t device_infor;
/****************************************************/

#define DEBUG 1

#if DEBUG
typedef struct
{
	unsigned char msg_type;
	PALLET_StateTypeDef prestate;
	PALLET_StateTypeDef curstate;

}debug_t;
PALLET_StateTypeDef pre = 0;
debug_t debug;
Debug_Queue_Typedef DebugQ;
//int tet;
void debug_enqueue(unsigned char type)
{
	debug.msg_type = type;
	debug.prestate = pre;
	debug.curstate = pallet_info.state;
	DebugQueue_Push(&DebugQ, (unsigned char*)&debug, 5);
}

#endif

_attribute_ram_code_  void Pallet_Setup_With_Gatway(Msg_TypeDef *msg);

void Pallet_Init(void)
{

	unsigned short addr;

	addr = Get_MAC_Addr();
	if(addr==0xffff)
		addr = Rand();

    pallet_info.mac_addr = addr;
    pallet_info.state = GP_SETUP_IDLE;
    pallet_info.p_gp_Setup_infor = &plt_setup_infor;
    pallet_info.pData = (void*)&node_data;

    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    RF_RxBufferSet(rx_buf, RX_BUF_LEN, 0);
    IRQ_RfIrqDisable(0xffff);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX);//
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    RF_SetTxRxOff();

#if DEBUG
    DebugQueue_Reset(&DebugQ);
#endif
}


_attribute_ram_code_ void Run_Pallet_Statemachine(Msg_TypeDef *msg)
{
    switch (pallet_info.state)
    {
		case GPN_CONN_IDLE:
		{
			temp_t0 = ClockTime();
			pallet_info.state = GPN_CONN_GW_BCN_WAIT;

			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
			break;
		}
		case GPN_CONN_GW_BCN_WAIT:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				pallet_info.state = GPN_CONN_SUSPEND_BEFORE_GB;
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
				pallet_info.t0 = pallet_info.wakeup_tick;
			}
			else if (msg && (msg->type == PALLET_MSG_TYPE_GW_BCN))
			{
				RX_INDICATE();
				//pallet_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
				pallet_info.gw_sn = FRAME_GET_DSN(msg->data);
				pallet_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data));

				if ((pallet_info.gw_sn % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM))
				{
	            	send_len = RF_Manual_Send(Build_PalletData, (void*)&pallet_info);
	            	TX_INDICATE();
	            	temp_t0 = ClockTime();
	            	pallet_info.state = GPN_CONN_PLT_DATA_TX_DONE_WAIT;
					memset(&node_data, 0, sizeof(NodeDataWaitSend_Typdedef)*3 );

				}
				else
				{
					pallet_info.state = GPN_CONN_SUSPEND_BEFORE_PB;
					pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
				}
				Message_Reset(msg);
			}
			break;
		}
		case GPN_CONN_PLT_DATA_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
    			pallet_info.state = GPN_CONN_GW_ACK_WAIT;
    			temp_t0 = ClockTime();
    			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case GPN_CONN_GW_ACK_WAIT:
		{
			//todo
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				pallet_info.state = GPN_CONN_SUSPEND_BEFORE_PB;
				pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
			}
			else if (msg && (msg->type == PALLET_MSG_TYPE_GW_ACK))
			{
				//RX_INDICATE();
				IRQ_INDICATION();
				pallet_info.state = GPN_CONN_SUSPEND_BEFORE_PB;
				pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
				Message_Reset(msg);
			}
			break;
		}
		case GPN_CONN_SUSPEND_BEFORE_PB:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
		#ifdef SUPEND
			//PrepareSleep();
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
		#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
		#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = GPN_CONN_SEND_PB;

			break;
		}
		case GPN_CONN_SEND_PB:
		{
			send_len = RF_Manual_Send(Build_PalletBeacon, (void*)&pallet_info);
			TX_INDICATE();
			temp_t0 = ClockTime();
			pallet_info.state = GPN_CONN_PB_TX_DONE_WAIT;
			break;
		}
		case GPN_CONN_PB_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				pallet_info.state = GPN_CONN_ND_DATA_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case GPN_CONN_ND_DATA_WAIT:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				pallet_info.state = GPN_CONN_SUSPEND_BEFORE_GB;
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD)*TickPerUs;
			}
			else if (msg)
			{
				if (msg->type == PALLET_MSG_TYPE_ED_DATA)
				{
					unsigned char node_id;
					RX_INDICATE();
					//ToDo: process received data submitted by end device
					//save the dsn for subsequent ack
					pallet_info.ack_dsn = msg->data[15];
					node_id = FRAME_ND_DATE_GET_SRC_ND_ID(msg->data);

					node_data[(node_id-1)%NODE_NUM].updata = 1;
					node_data[(node_id-1)%NODE_NUM].temperature = FRAME_ND_DATE_GET_ND_PAYLOAD(msg->data);
					pallet_info.state = GPN_CONN_SEND_ND_ACK;
				}
				Message_Reset(msg);
			}
			break;
		}
		case GPN_CONN_SEND_ND_ACK:
		{
        	send_len = RF_Manual_Send(Build_Ack, (void*)&pallet_info.ack_dsn);
        	TX_INDICATE();
        	temp_t0 = ClockTime();
        	pallet_info.state = GPN_CONN_ND_ACK_TX_DONE_WAIT;
			break;
		}
		case GPN_CONN_ND_ACK_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
    			TX_INDICATE();

    			pallet_info.state = GPN_CONN_SUSPEND_BEFORE_GB;
    			temp_t0 = ClockTime();
    			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
    			pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD)*TickPerUs;
			}

			break;
		}
		case GPN_CONN_SUSPEND_BEFORE_GB:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
	#ifdef SUPEND
			//PrepareSleep();
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
	#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
	#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = GPN_CONN_IDLE;
			break;
		}

		default :
			break;
    }
}


#if DEBUG
	static unsigned char pack[8][32];
	static unsigned char dj;
#endif
_attribute_ram_code_ void Pallet_RxIrqHandler(void)
{
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);


    if ((rx_packet[13] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet))) {
        // Garbage packet
        MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else
	{
#if DEBUG
    	if(pallet_info.state==GPN_CONN_GW_BCN_WAIT)
    	{
    	if(dj>=8)
    		dj = 0;
    	memcpy(pack[dj++], rx_packet, 32);
    	}
#endif
    	unsigned int tick = ClockTime();
    	rx_packet[8] = (unsigned char)(tick & 0xff);
    	rx_packet[9] = (unsigned char)((tick>>8) & 0xff);
    	rx_packet[10] = (unsigned char)((tick>>16) & 0xff);
    	rx_packet[11] = (unsigned char)((tick>>24) & 0xff);

        //if it is coor ACK frame, check it and then go to suspend immediately
        if (FRAME_IS_ACK_TYPE(rx_packet) && (rx_packet[15] == pallet_info.dsn))
		{
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_GW_ACK);
        }
        //if it is node setup request
        else if (FRAME_IS_SETUP_NODE_REQ(rx_packet))
		{
            if (pallet_info.mac_addr == FRAME_GET_DST_ADDR(rx_packet))
			{
                MsgQueue_Push(&msg_queue, rx_packet, PN_MSG_ND_SETUP_REQ);
            }
            else {
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
            }
        }
        //if it is coor BCN frame, do sync and report data if it is this device's opportunity and go to suspend otherwise
        else if (FRAME_IS_GATEWAY_BEACON(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_GW_BCN);
        }
        //if it is end device data frame, send ack immediately
        else if (FRAME_IS_NODE_DATA(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_ED_DATA);
        }
        //if it is gateway setup beacon
        else if (FRAME_IS_SETUP_GW_BEACON(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, PG_MSG_SETUP_GW_BCN);
        }
        //if it is gateway setup response frame, check whether the dst addr matches the local addr
        else if (FRAME_IS_SETUP_GW_RSP(rx_packet))
        {
            unsigned short dst_addr = FRAME_GET_DST_ADDR(rx_packet);
            if (dst_addr == pallet_info.mac_addr)
            {
                MsgQueue_Push(&msg_queue, rx_packet, PG_MSG_SETUP_GW_RSP);
            }
            else
            {
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
            }
        }
        else
        {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
        }
    }
}


_attribute_ram_code_ void Pallet_TxDoneHandle(void)
{
	MsgQueue_Push(&msg_queue, NULL, MSG_TX_DONE);
}

_attribute_ram_code_ void Run_Pallet_Setup_With_Node(Msg_TypeDef *msg)
{
    switch(pallet_info.state)
    {
		case PN_SETUP_IDLE:
		{
			if(pallet_info.dsn>=PLT_SETUP_BCN_NUM)
			{
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD*2 - 300)*TickPerUs;
				pallet_info.t0 = pallet_info.wakeup_tick;
				pallet_info.state = GPN_CONN_SUSPEND_BEFORE_GB;
			}
			else
			{
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
				pallet_info.state = PN_SETUP_GW_BCN_WAIT;
			}

			break;
		}
		case PN_SETUP_GW_BCN_WAIT:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
            	pallet_info.state = PN_SETUP_SUSPEND;
            	pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD *TickPerUs;
            	pallet_info.t0 = pallet_info.wakeup_tick;
			}
			if (msg)
			{
                if (msg->type == PALLET_MSG_TYPE_GW_BCN)
                {
                	RX_INDICATE();
                	//pallet_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                	pallet_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data));

                	pallet_info.gw_sn = FRAME_GET_DSN(msg->data);
                	pallet_info.wakeup_tick = pallet_info.t0 + GW_PLT_TIME *TickPerUs;
                	pallet_info.state = PN_SETUP_SUSPEND_BEFORE_SEND_BCN;
                }
                Message_Reset(msg);
			}
			break;
		}
		case PN_SETUP_SUSPEND_BEFORE_SEND_BCN:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
#if SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = PN_SETUP_SEND_BCN;
			break;
		}
		case PN_SETUP_SEND_BCN:
		{
			send_len = RF_Manual_Send(Build_PalletSetupBeacon, (void*)&pallet_info);
			TX_INDICATE();
			temp_t0 = ClockTime();
			pallet_info.state = PN_SETUP_SEND_BCN_TX_DONE_WAIT;
			break;
		}
		case PN_SETUP_SEND_BCN_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				pallet_info.state = PN_SETUP_ND_REQ_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case PN_SETUP_ND_REQ_WAIT:
		{
			if(ClockTime() - (pallet_info.t0 + MASTER_PERIOD *TickPerUs)<BIT(31))
			{
				pallet_info.state = PN_SETUP_IDLE;
			}
			else
			{
				if((msg) && (msg->type==PN_MSG_ND_SETUP_REQ))
				{
					unsigned short dest_mac = FRAME_GET_DST_ADDR(msg->data);

	            	if(dest_mac == pallet_info.mac_addr)
	            	{
	            		unsigned int i;
	            		RX_INDICATE();
						//send pallet setup response

						pallet_info.node_addr = FRAME_GET_SRC_ADDR(msg->data);
						//check whether the node has been added in to the node table
						for (i = 0; i < pallet_info.node_table_len; i++)
						{
							if (pallet_info.node_addr == node_table[i].node_addr)
							{
								pallet_info.node_id = node_table[i].node_id;
								break;
							}
						}
						if (i == pallet_info.node_table_len)
						{
							pallet_info.node_table_len++;
							pallet_info.node_id = pallet_info.node_table_len;
							//add the new node to node table
							assert(pallet_info.node_table_len <= NODE_TABLE_MAX_LEN);
							node_table[i].node_addr = pallet_info.node_addr;
							node_table[i].node_id = pallet_info.node_id;
						}
						send_len = RF_Manual_Send(Build_PalletSetupRsp, (void*)&pallet_info);
						TX_INDICATE();
						temp_t0 = ClockTime();
						pallet_info.state = PN_SETUP_ND_RSP_TX_DONE;
					}

					Message_Reset(msg);
				}

			}
			break;
		}
		case PN_SETUP_ND_RSP_TX_DONE:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				pallet_info.state = PN_SETUP_ND_REQ_WAIT;
			}
			break;
		}
		case PN_SETUP_SUSPEND:
		{
			GPIO_WriteBit(POWER_PIN, 0);
			RF_SetTxRxOff();
#if SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = PN_SETUP_IDLE;
			break;
		}
		default:
			break;
    }
}



_attribute_ram_code_  void Pallet_Setup_With_Gatway(Msg_TypeDef *msg)
{
    static unsigned int now;
    static unsigned char conn_falg=0;

    switch (pallet_info.state)
    {
    	case PALLET_STATE_OFF:
    	{
    		RF_SetTxRxOff();
    		ERROR_WARN_LOOP();
    		break;
    	}
    	case GP_SETUP_IDLE:
    	{
    		temp_t0 = ClockTime();
    		pallet_info.state = GP_SETUP_BCN_WAIT;
    		RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
    		break;
    	}
    	case GP_SETUP_BCN_WAIT:
    	{
        	now = ClockTime();
        	if(now - (temp_t0 + TIMESLOT_LENGTH*PALLET_NUM*5*TickPerUs)<BIT(31))//长时间没有收到包，reset RF baseband
        	{//reset RF baseband state
        		pallet_info.state = GP_SETUP_IDLE;
        	}
            if (msg)
            {
                if (msg->type == PG_MSG_SETUP_GW_BCN)
                {
                	RX_INDICATE();

                    plt_setup_infor.gw_mac = FRAME_GET_SRC_ADDR(msg->data);
                    plt_setup_infor.gw_id = FRAME_GW_SETUP_GB_SRC_ID(msg->data);
                    //pallet_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                    pallet_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data));
                    if(conn_falg==1)
                    {
                    	if((plt_setup_infor.gw_mac==pallet_info.gw_addr) && (plt_setup_infor.gw_id==pallet_info.gw_id))
                    	{
                    		pallet_info.state = GP_SETUP_SUSPEND;
                    		pallet_info.wakeup_tick  = pallet_info.t0 + MASTER_PERIOD*TickPerUs;
                    		break;
                    	}
                    }
                    else
                    {
                    	pallet_info.state = GP_SETUP_BACKOFF;
                        pallet_info.wakeup_tick = ClockTime() +  (((Rand() % (MASTER_PERIOD - ND_WAIT_BCN_MARGIN)))/BACKOFF_UNIT)*BACKOFF_UNIT*TickPerUs;
                    }

                }
                else if((msg->type == PALLET_MSG_TYPE_GW_BCN) && (conn_falg==1))
                {
                	if((pallet_info.gw_addr == FRAME_GW_GB_GET_GW_ADDR(msg->data)) && (pallet_info.gw_id==FRAME_GW_GB_GET_GW_ID(msg->data)))
                	{
                		//GPIO_SetBit(LED3_RED);
                		pallet_info.state = GP_SYC_SUSPNED;
                		pallet_info.wakeup_tick  = pallet_info.t0 + MASTER_PERIOD*TickPerUs;
                	}
                }
                Message_Reset(msg);
            }

    		break;
    	}
    	case GP_SETUP_BACKOFF:
    	{
    		RF_SetTxRxOff();
    		GPIO_WriteBit(POWER_PIN, 0);
    #if SUPEND
    		PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
    #else
    		while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
    #endif
    		GPIO_WriteBit(POWER_PIN, 1);

            pallet_info.state = GP_SETUP_REQ_SEND;
    		break;
    	}
    	case GP_SETUP_REQ_SEND:
    	{
            if (plt_setup_infor.retry_times < RETRY_MAX)
            {
            	send_len = RF_Manual_Send(Build_PalletSetupReq, (void*)&pallet_info);
            	TX_INDICATE();
            	temp_t0 = ClockTime();

            	pallet_info.state = GP_SETUP_REQ_TX_DONE_WAIT;
            }
            else
            {
            	pallet_info.state = PALLET_STATE_OFF;
            }
    		break;
    	}
    	case GP_SETUP_REQ_TX_DONE_WAIT:
    	{
    		if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
    		{
    			TX_INDICATE();

    			pallet_info.state = GP_SETUP_GW_RSP_WAIT;
    			temp_t0 = ClockTime();
    			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
    		}
    		break;
    	}
    	case GP_SETUP_GW_RSP_WAIT:
    	{
    		if(ClockTimeExceed(temp_t0, RX_WAIT))
    		{
    			RX_INDICATE();

                plt_setup_infor.gw_mac = 0;
                plt_setup_infor.gw_id = 0;
                plt_setup_infor.retry_times ++;

                pallet_info.state = GP_SETUP_SUSPEND;
                pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*TickPerUs;
                pallet_info.t0 = pallet_info.wakeup_tick;

    		}
    		if((msg) && (msg->type == PG_MSG_SETUP_GW_RSP))
            {
				//need match mac

				if((plt_setup_infor.gw_mac==FRAME_GW_SETUP_RSP_GET_GW_SRC_ADDR(msg->data)) && (pallet_info.mac_addr == FRAME_GW_SETUP_RSP_GET_PLT_ADDR(msg->data)))
				{
					RX_INDICATE();

					pallet_info.state = GP_SETUP_SUSPEND;
					conn_falg = 1;
					pallet_info.gw_addr = plt_setup_infor.gw_mac;
					pallet_info.gw_id = plt_setup_infor.gw_id;
					pallet_info.pallet_id = FRAME_GW_SETUP_RSP_GET_PLT_ID(msg->data);
					pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*TickPerUs;

					CONN_INDICATION();
	                Message_Reset(msg);
				}
            }
    		break;
    	}
    	case GP_SETUP_SUSPEND:
    	{
    		RF_SetTxRxOff();
    		GPIO_WriteBit(POWER_PIN, 0);
    #if SUPEND
    		PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);
    #else
    		while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
    #endif
    		GPIO_WriteBit(POWER_PIN, 1);
    		pallet_info.state = GP_SETUP_IDLE;
    		break;
    	}
    	default:
    		break;
    }
}

_attribute_ram_code_  void Pallet_Keep_Syc_With_GW(Msg_TypeDef *msg)
{
	switch(pallet_info.state)
	{
		case GP_SYC_IDLE:
		{
			temp_t0 = ClockTime();
			pallet_info.state = GP_SYC_LISTEN_GB;
			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
		}
		case GP_SYC_LISTEN_GB:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				TIME_INDICATE();
				pallet_info.state = GP_SYC_SUSPNED;
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
				pallet_info.t0 = pallet_info.wakeup_tick;
			}
			else if(msg)
			{
				if(msg->type==PALLET_MSG_TYPE_GW_BCN)
				{
					RX_INDICATE();
		            pallet_info.t0 = Estimate_SendT_From_RecT(FRAME_GET_TIMESTAMP(msg->data), FRAME_GET_LENGTH(msg->data));//-150*TickPerUs
		            //pallet_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
		            pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*TickPerUs;
		            pallet_info.gw_sn = (FRAME_GET_DSN(msg->data));
#if 1
		            if ((pallet_info.gw_sn % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM))
		            {
		            	send_len = RF_Manual_Send(Build_PalletData, (void*)&pallet_info);
		            	TX_INDICATE();
		            	temp_t0 = ClockTime();
		            	pallet_info.state = GP_SYC_PLT_DATA_TX_DONE_WAIT;
		            }
		            else
		            {
		            	pallet_info.state = GP_SYC_SUSPNED;
		            }
#endif
				}
				else
				{
					pallet_info.state = GP_SYC_SUSPNED;
					pallet_info.wakeup_tick = pallet_info.wakeup_tick + MASTER_PERIOD*TickPerUs;
				}
				Message_Reset(msg);
			}
			else
			{

			}
			break;
		}
		case GP_SYC_PLT_DATA_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				pallet_info.state = GP_SYC_ACK_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
		case GP_SYC_ACK_WAIT:
		{
			if(ClockTimeExceed(temp_t0, RX_WAIT))
			{
				TIME_INDICATE();
				pallet_info.state = GP_SYC_SUSPNED;
			}
			else if(msg)
            {
				if (msg->type == PALLET_MSG_TYPE_GW_ACK)
				{

					RX_INDICATE();
					pallet_info.is_associate = 1;
					pallet_info.state = GP_SYC_SUSPNED;
					ACK_REC_INDICATION();
				}
				Message_Reset(msg);
            }
			break;
		}
		case GP_SYC_SUSPNED:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
#if SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);

#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
#endif
			GPIO_WriteBit(POWER_PIN, 1);

			if(pallet_info.node_table_len != 0)
			{
				pallet_info.state = GPN_CONN_IDLE;
			}
			else
			{
				pallet_info.state = GP_SYC_IDLE;
			}

			break;
		}
		default:
			break;
	}
}

void Pallet_MainLoop(void)
{
    Msg_TypeDef *pMsg = NULL;

#if DEBUG
	pre = pallet_info.state;
#endif
    if((PalletSetupTrig==1) &&(pallet_info.is_associate == 1))
    {
    	PalletSetupTrig = 0;
    	pallet_info.dsn = 0;
    	pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD*2)*TickPerUs;
    	pallet_info.t0 = pallet_info.wakeup_tick;
    	pallet_info.state = PN_SETUP_SUSPEND;
    	GPIO_WriteBit(TEST_PIN, !GPIO_ReadOutputBit(TEST_PIN));
    }

    pMsg = MsgQueue_Pop(&msg_queue);

    if(IS_STATE_PALLET_SETUP_WITH_GATEWAY(pallet_info.state))
    {
    	Pallet_Setup_With_Gatway(pMsg);
    }
    else if(IS_STATE_PALLET_KEEP_SYC_WITH_GW(pallet_info.state))
    {
    	Pallet_Keep_Syc_With_GW(pMsg);
    }
    else if(IS_STATE_PALLET_SETUP_WITH_NODE(pallet_info.state))
    {
    	Run_Pallet_Setup_With_Node(pMsg);
    }
    //run consign
    else if(IS_STATE_PALLET_CONSIGN(pallet_info.state))
    {
    	 Run_Pallet_Statemachine(pMsg);
    }

#if DEBUG
    if(pre != pallet_info.state)
    {
    	if(pMsg!= NULL)
    		debug_enqueue(pMsg->type );
    	else
    		debug_enqueue(MSG_TYPE_NONE );
    }
#endif
}
