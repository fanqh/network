#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "pallet.h"
#include "../debug/debug_queue.h"

#define NODE_TABLE_MAX_LEN   6

PalletInfo_TypeDef pallet_info;
static MsgQueue_Typedef msg_queue;

static NodeEntry_Typedef node_table[NODE_TABLE_MAX_LEN];

static volatile unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static volatile unsigned char rx_ptr = 0;
volatile unsigned char PalletSetupTrig = 0;
unsigned int temp_t1;



static int Pallet_SetupTimer_Callback(void *data);
_attribute_ram_code_  void Pallet_Setup_With_Gatway(Msg_TypeDef *msg);
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
unsigned char Wait_Tx_Done(unsigned int timeout);


void Get_Pallet_Infor(device_infor_t *pInfor)
{
	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(device_infor_t), (unsigned char*)&device_infor);
	if(device_infor.pallet_mac == 0xffff)
		device_infor.pallet_mac = PALLET_MAC_ADDR;
	if(device_infor.pallet_id == 0xffff)
		device_infor.pallet_id = 1;
}

void Pallet_Init(void)
{

    memset(&pallet_info, 0, sizeof(PalletInfo_TypeDef));

    Get_Pallet_Infor(&device_infor);

    pallet_info.gw_id = 1;
    pallet_info.dsn = pallet_info.mac_addr & 0xff;
    pallet_info.mac_addr = device_infor.pallet_mac;
    pallet_info.pallet_id = device_infor.pallet_id;
    pallet_info.state = GP_SETUP_IDLE;


    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    //set rx buffer
    RF_RxBufferSet(rx_buf, RX_BUF_LEN, 0);
    //enable irq
    IRQ_RfIrqDisable(FLD_RF_IRQ_TX);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_FIRST_TIMEOUT);
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_Enable();
    RF_SetTxRxOff();

#if DEBUG
    DebugQueue_Reset(&DebugQ);
#endif
}


_attribute_ram_code_ void Run_Pallet_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;

    switch (pallet_info.state)
    {
		case PALLET_STATE_IDLE:
		{
			temp_t1 = ClockTime();
			pallet_info.state = PALLET_STATE_GW_BCN_WAIT;

			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
			break;
		}
		case PALLET_STATE_GW_BCN_WAIT:
		{
			if(ClockTimeExceed(temp_t1, RX_WAIT))
			{
				pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
				pallet_info.t0 = pallet_info.wakeup_tick;
			}
			else if (msg && (msg->type == PALLET_MSG_TYPE_GW_BCN))
			{
				GPIO_WriteBit(SHOW_DEBUG, !GPIO_ReadOutputBit(SHOW_DEBUG));
				now = ClockTime();
				pallet_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
				pallet_info.gw_sn = FRAME_GET_DSN(msg->data);

				if ((pallet_info.gw_sn % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM))
				{
					TIME_INDICATE();

					RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
					Build_PalletData(tx_buf, &pallet_info, node_data);
					RF_TxPkt(tx_buf);
					Wait_Tx_Done(3000);

					temp_t1 = ClockTime();
					pallet_info.state = PALLET_STATE_GW_ACK_WAIT;
					RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on waiting for mesh setup beacon

					memset(&node_data, 0, sizeof(NodeDataWaitSend_Typdedef)*3 );

				}
				else
				{
					pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
					pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
				}
				Message_Reset(msg);
			}
			break;
		}
		case PALLET_STATE_GW_ACK_WAIT:
		{
			//todo
			if(ClockTimeExceed(temp_t1, RX_WAIT))
			{
				TIME_INDICATE();
				pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
				pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
			}
			else if (msg)
			{
				TIME_INDICATE();
				pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
				pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
				if (msg->type == PALLET_MSG_TYPE_GW_ACK)
				{
					//todo
				}
				else
				{
					//todo
				}
				Message_Reset(msg);
			}
			break;
		}
		case PALLET_STATE_SUSPEND_BEFORE_PB:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
		#ifdef SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
		#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
		#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = PALLET_STATE_SEND_PB;

			break;
		}
		case PALLET_STATE_SEND_PB:
		{

			RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
			Build_PalletBeacon(tx_buf, &pallet_info);
			TIME_INDICATE();
			RF_TxPkt(tx_buf);
			Wait_Tx_Done(TX_DONE_TIMEOUT);

			temp_t1 = ClockTime();
			RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			pallet_info.state = PALLET_STATE_NODE_DATA_WAIT;

			break;
		}
		case PALLET_STATE_NODE_DATA_WAIT:
		{
			if(ClockTimeExceed(temp_t1, RX_WAIT))
			{
				TIME_INDICATE();
				pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
			}
			else if (msg)
			{
				if (msg->type == PALLET_MSG_TYPE_ED_DATA)
				{
					unsigned char node_id;
					TIME_INDICATE();
					GPIO_WriteBit(LED3_RED, !GPIO_ReadOutputBit(LED3_RED));
					//ToDo: process received data submitted by end device
					//save the dsn for subsequent ack
					pallet_info.ack_dsn = msg->data[15];
					node_id = FRAME_GET_SRC_NODE_ID(msg->data);

					node_data[(node_id-1)%NODE_NUM].updata = 1;
					node_data[(node_id-1)%NODE_NUM].temperature = FRAME_GET_NODE_PAYLOAD(msg->data);
					pallet_info.state = PALLET_STATE_SEND_NODE_ACK;
				}
				Message_Reset(msg);
			}
			break;
		}
		case PALLET_STATE_SEND_NODE_ACK:
		{
			RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
			Build_Ack(tx_buf, pallet_info.ack_dsn);
			TIME_INDICATE();
			RF_TxPkt(tx_buf);
			Wait_Tx_Done(TX_DONE_TIMEOUT);
			TIME_INDICATE();
			pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
			pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
			break;
		}
		case PALLET_STATE_SUSPEND_BEFORE_GB:
		{
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
	#ifdef SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
	#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
	#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = PALLET_STATE_IDLE;
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
    	if(pallet_info.state==PALLET_STATE_GW_BCN_WAIT)
    	{
    	if(dj>=8)
    		dj = 0;
    	memcpy(pack[dj++], rx_packet, 32);
    	}
#endif

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
        else if (FRAME_IS_NODE_DATA(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_ED_DATA);
        }
        //if it is gateway setup beacon
        else if (FRAME_IS_SETUP_GW_BEACON(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_SETUP_GW_BCN);
        }
        //if it is gateway setup response frame, check whether the dst addr matches the local addr
        else if (FRAME_IS_SETUP_GW_RSP(rx_packet)) {
            unsigned short dst_addr = FRAME_GET_DST_ADDR(rx_packet);
            if (dst_addr == pallet_info.mac_addr) {
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_SETUP_GW_RSP);
            }
            else {
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
            }
        }
        else {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
        }
    }
}

_attribute_ram_code_ void Pallet_RxTimeoutHandler(void)
{

	if(pallet_info.state & PN_SETUP_STATE_MASK)
	{
		MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_ED_DATA_TIMEOUT);
	}
	else if(pallet_info.state & GP_SETUP_STATE_MASK)
	{
		MsgQueue_Push(&msg_queue, NULL, MSG_P_SETUP_WAIT_G_TIMEOUT);
	}
	else if(pallet_info.state & GP_KEEP_SYC_MASK)
	{
		MsgQueue_Push(&msg_queue, NULL, MSG_P_SETUP_WAIT_G_TIMEOUT);
	}
	else if(pallet_info.state & GP_KEEP_SYC_MASK)
	{

	}
	else
	{

	}
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
	RF_TxFinishClearFlag();
	return SUCCESS;
}

_attribute_ram_code_ void Run_Pallet_Setup_With_Node(Msg_TypeDef *msg)
{
    unsigned int now;
    unsigned char i;

    switch(pallet_info.state)
    {
		case PN_SETUP_IDLE:
		{
			if(pallet_info.dsn>=PLT_SETUP_BCN_NUM)
			{
				GPIO_WriteBit(TEST_PIN, !GPIO_ReadOutputBit(TEST_PIN));
				pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD*2 - 300)*TickPerUs;
				pallet_info.t0 = pallet_info.wakeup_tick;
				pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
			}
			else
			{
				temp_t1 = ClockTime();
				//pallet_info.t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
				pallet_info.state = PN_SETUP_GW_BCN_WAIT;
			}

			break;
		}
		case PN_SETUP_GW_BCN_WAIT:
		{
			if(ClockTimeExceed(temp_t1, RX_WAIT))
			{
            	pallet_info.state = PN_SETUP_SUSPEND;
            	pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD *TickPerUs;
			}
			if (msg)
			{
                if (msg->type == PALLET_MSG_TYPE_GW_BCN)
                {
                	unsigned int timestamp;

                	GPIO_WriteBit(SHOW_DEBUG, !GPIO_ReadOutputBit(SHOW_DEBUG));
                	timestamp = FRAME_GET_TIMESTAMP(msg->data);
                	pallet_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
                	pallet_info.gsn = FRAME_GET_GB_DSN(msg->data);
                	pallet_info.wakeup_tick = pallet_info.t0 + GW_PLT_TIME *TickPerUs;
                	pallet_info.state = PN_SETUP_SUSPEND_BEFORE_SEND_BCN;
                }
#if 0
                else if(msg->type == PALLET_MSG_TYPE_ED_DATA_TIMEOUT)
                {
                	pallet_info.state = PN_SETUP_SUSPEND;
                	pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD *TickPerUs;
                }
                else//invaliad data
                {
                	pallet_info.state = PN_SETUP_SUSPEND;
                	pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD *TickPerUs;
                }
#endif
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
	        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
	        Build_PalletSetupBeacon(tx_buf, &pallet_info);
	        TIME_INDICATE();
	        RF_TxPkt(tx_buf);
	        if(Wait_Tx_Done(TX_DONE_TIMEOUT)!=SUCCESS)
	        {
	        	ERROR_WARN_LOOP();
	        }
	        TIME_INDICATE();

	        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
	        pallet_info.state = PN_SETUP_NODE_REQ_WAIT;
			break;
		}
		case PN_SETUP_NODE_REQ_WAIT:
		{
			if(ClockTime() - (pallet_info.t0 + MASTER_PERIOD *TickPerUs)<BIT(31))
				pallet_info.state = PN_SETUP_IDLE;
			else
			{
				if((msg) && (msg->type==PN_MSG_ND_SETUP_REQ))
				{
					unsigned short dest_mac = FRAME_GET_DST_ADDR(msg->data);
	            	if(dest_mac == pallet_info.mac_addr)
	            	{
	            		unsigned int i;

	            		TIME_INDICATE();
						//send pallet setup response
						RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
						pallet_info.node_addr = FRAME_GET_SRC_ADDR(msg->data);
						//check whether the node has been added in to the node table
						for (i = 0; i < pallet_info.node_table_len; i++)
						{
							if (pallet_info.node_addr == node_table[i].node_addr) {
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
						Build_PalletSetupRsp(tx_buf, &pallet_info);
						RF_TxPkt(tx_buf);
						Wait_Tx_Done(TX_DONE_TIMEOUT);
						TIME_INDICATE();
						//RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //resume to rx mode and continue to receive PALLET_MSG_TYPE_SETUP_REQ
					}

					Message_Reset(msg);
				}

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
			pallet_info.state = PN_SETUP_IDLE;
			GPIO_WriteBit(POWER_PIN, 1);
			break;
		}
    }
}



_attribute_ram_code_  void Pallet_Setup_With_Gatway(Msg_TypeDef *msg)
{
    static unsigned int now,rx_begain_t0, gwb_receive_t0;

    switch (pallet_info.state)
    {
    	case PALLET_STATE_OFF:
    	{
    		RF_SetTxRxOff();
    		ERROR_WARN_LOOP();
    	}
    	case GP_SETUP_IDLE:
    	{
    		rx_begain_t0 = ClockTime();
    		RF_SetTxRxOff();
    		pallet_info.state = GP_SETUP_BCN_WAIT;
    		RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
    		break;
    	}
    	case GP_SETUP_BCN_WAIT:
    	{
        	now = ClockTime();
//        	if(now - (rx_begain_t0 + TIMESLOT_LENGTH*PALLET_NUM*5*TickPerUs)<BIT(31))//长时间没有收到包，reset RF baseband
//        	{//reset RF baseband state
//        		pallet_info.state = GP_SETUP_IDLE;
//        	}

            if (msg)
            {
                if (msg->type == PALLET_MSG_TYPE_SETUP_GW_BCN)
                {
                	TIME_INDICATE();
                	gwb_receive_t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                    pallet_info.state = GP_SETUP_BACKOFF;
                    pallet_info.retry_times = 0;
                    pallet_info.t0 = FRAME_GET_TIMESTAMP(msg->data) - ZB_TIMESTAMP_OFFSET*TickPerUs;
                    pallet_info.gw_setup_bcn_total = FRAME_GATEWYA_SETUP_BCN_TOTAL_NUM(msg->data);
                    pallet_info.gw_sn = (FRAME_GET_DSN(msg->data));
                    if((pallet_info.gw_setup_bcn_total>=500) || (pallet_info.gw_sn >=pallet_info.gw_setup_bcn_total - 6))
                    	pallet_info.state = PALLET_STATE_OFF;

                    pallet_info.gw_addr = FRAME_GET_SRC_ADDR(msg->data);
                    TIME_INDICATE();

                }
                Message_Reset(msg);
            }
    		break;
    	}
    	case GP_SETUP_BACKOFF:
    	{
    		//todo gateway setup 的总数可以放在gateway包中
            pallet_info.wakeup_tick = now + (((Rand()& BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs) % ((pallet_info.gw_setup_bcn_total - pallet_info.gw_sn)*TIMESLOT_LENGTH*TickPerUs));
            PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);

            pallet_info.state = GP_SETUP_REQ_SEND;
    		break;
    	}
    	case GP_SETUP_REQ_SEND:
    	{
            if (pallet_info.retry_times < RETRY_MAX)
            {
            	//TIME_INDICATE();
                now = ClockTime();
                RF_SetTxRxOff();
                RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL);
                RF_StartStxToRx(tx_buf, now + RF_TX_WAIT*TickPerUs, RX_WAIT);
                Build_PalletSetupReq(tx_buf, &pallet_info);

                pallet_info.state = GP_SETUP_GW_RSP_WAIT;
            }
            else
            {
            	pallet_info.state = PALLET_STATE_OFF;
            }
    		break;
    	}
    	case GP_SETUP_GW_RSP_WAIT:
    	{
            if (msg)
            {
                if (msg->type == PALLET_MSG_TYPE_SETUP_GW_RSP)
                {
                	//need match mac
                	TIME_INDICATE();
                	pallet_info.is_associate = 1;
                    pallet_info.state = GP_SYC_SUSPNED;
                    pallet_info.pallet_id = FRAME_GET_PALLET_ID_FROM_GATEWAY_SETUP(msg->data);
                    pallet_info.wakeup_tick = pallet_info.t0 + (pallet_info.gw_setup_bcn_total-pallet_info.gw_sn)*TIMESLOT_LENGTH*PALLET_NUM*TickPerUs;
                    //ERROR_WARN_LOOP();
                    TIME_INDICATE();
                    GPIO_SetBit(LED1_GREEN);
                }
                else
                {
                	//TIME_INDICATE();
                    now = ClockTime();
                    pallet_info.state = GP_SETUP_BCN_WAIT;
                    //pallet_info.wakeup_tick = now + (((Rand()& BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs) % ((pallet_info.gw_setup_bcn_total - pallet_info.gw_setup_sn)*TIMESLOT_LENGTH*TickPerUs));
                    pallet_info.retry_times++;
                }

                Message_Reset(msg);
            }
    		break;
    	}
    	default:
    		break;
    }
}

_attribute_ram_code_  void Pallet_Keep_Syc_With_GW(Msg_TypeDef *msg)
{
	unsigned int now;

	switch(pallet_info.state)
	{
		case GP_SYC_LISTEN_GB:
		{
#if 1
			if(msg)
			{
				if(msg->type==PALLET_MSG_TYPE_GW_BCN)
				{
					//GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
		            now = ClockTime();
		            unsigned int timestamp = FRAME_GET_TIMESTAMP(msg->data);
		            pallet_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
		            //pallet_info.t0 = now;
		            GPIO_WriteBit(SHOW_DEBUG, !GPIO_ReadOutputBit(SHOW_DEBUG));
		            pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*TickPerUs;

		            pallet_info.gw_sn = (FRAME_GET_DSN(msg->data));
#if 1
		            if ((pallet_info.gw_sn % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM))
		            {
		            	TIME_INDICATE();
		                RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL);
		                RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
		                Build_PalletData(tx_buf, &pallet_info, node_data);
		                //clear data that have been sent
						memset(&node_data, 0, sizeof(NodeDataWaitSend_Typdedef)*3 );
		                pallet_info.state = GP_SYC_ACK_WAIT;
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
#endif
			break;
		}
		case GP_SYC_SUSPNED:
		{
			unsigned int rec_window_size = 0;
			RF_SetTxRxOff();
			GPIO_WriteBit(POWER_PIN, 0);
#if SUPEND
			PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick - SETUP_SUSPNED_EARLY_WAKEUP*TickPerUs);

#else
			while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
#endif
			GPIO_WriteBit(POWER_PIN, 1);
			pallet_info.state = GP_SYC_LISTEN_GB;
	        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //switch to auto mode
	        //todo 应该根据睡眠时间来设置窗口大小,提前醒来的时间，也应该对应接收窗口大小
	        if(pallet_info.gw_sn<=pallet_info.gw_setup_bcn_total)
	        	rec_window_size = SYC_EXTEND_WINDOW_SIZE;
	        else
	        	rec_window_size = SYC_WINDOW_SIZE;
	        RF_StartSrx(ClockTime(), rec_window_size);

			break;
		}
		case GP_SYC_ACK_WAIT:
		{
            if(msg)
            {
            	TIME_INDICATE();
            	pallet_info.state = GP_SYC_SUSPNED;
				if (msg->type == PALLET_MSG_TYPE_GW_ACK)
				{
					//todo
					GPIO_WriteBit(LED3_RED, !GPIO_ReadOutputBit(LED3_RED));
				}
				else if (msg->type == PALLET_MSG_TYPE_GW_ACK_TIMEOUT)
				{
					//todo
				}
				else if (msg->type == PALLET_MSG_TYPE_INVALID_DATA)
				{
					//todo
				}
				else
				{
					//todo
				}
				Message_Reset(msg);
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
    	//pallet_setup_timer = ev_on_timer(Pallet_SetupTimebea_Callback, NULL, PALLET_SETUP_PERIOD);
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
