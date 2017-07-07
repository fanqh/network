#include "../common.h"
#include "../drivers.h"
#include "config.h"
#include "frame.h"
#include "message_queue.h"
#include "gateway.h"
#include "../interface/pc_interface.h"
#include "mac.h"
#include "mac_data.h"

typedef struct
{
	unsigned char flag;
	unsigned char dsn;
	unsigned char pallet_id;
	unsigned int temperature;

}WaitForUpload_Typedef;

static unsigned int temp_t0 = 0;
static unsigned char send_len;

static MsgQueue_Typedef msg_queue;


static GWInfo_TypeDef gw_info;
static Conn_List_Typedef gw_conn_list;
static GatewaySetupInfor_Typedef gw_setup_infor;
//static Rec_Infor_Typedef rec_infor;

static volatile unsigned char rx_ptr = 0;
unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
extern volatile unsigned char GatewaySetupTrig;


WaitForUpload_Typedef DataToSend;

void Gateway_Init(void)
{

	unsigned short addr;

	addr = Get_MAC_Addr();
	if(addr == 0xffff)
		addr = Rand();

    gw_info.mac_addr = addr;
    gw_info.state = GW_STATE_RF_OFF;
	gw_info.gw_id = Rand()%255;
	gw_info.pSetup_info = &gw_setup_infor;
	gw_info.pConn_list = &gw_conn_list;

	Init_DataBase(&gw_conn_list);
    RF_SetTxRxOff();
    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
#if PA_MODE
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX);
#else
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_TX);
#endif
}

//_attribute_ram_code_ Refresh_Lost_ID_List(void)
//{
//	unsigned char i;
//
//	for(i=0; i< gw_conn_list.num; i++)
//}
_attribute_ram_code_ void Run_Gateway_Statemachine(Msg_TypeDef *msg)
{
	switch (gw_info.state)
	{
		case GW_CONN_SEND_GW_BCN:
		{
			send_len = RF_Manual_Send(Build_GatewayBeacon, (void*)&gw_info);
			TX_INDICATE();
			temp_t0 = ClockTime();
			gw_info.t0 = temp_t0;
			gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD)*TickPerUs;

			gw_info.state = GW_CONN_GB_TX_DONE;
			break;
		}
		case GW_CONN_GB_TX_DONE:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				gw_info.state = GW_CONN_PLT_DATA_WAIT;
				temp_t0 = ClockTime();
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
			}
			break;
		}
	case GW_CONN_PLT_DATA_WAIT:
    {
		if(ClockTime() - (gw_info.wakeup_tick -TIMESLOT_LENGTH*5*TickPerUs) <= BIT(31))
		{
			gw_info.state = GW_CONN_SUSPEND;
		}
		if (msg)
    	{
            if (msg->type == GW_MSG_TYPE_PALLET_DATA)
            {
            	RX_INDICATE();
                //ToDo: process received data submitted by pallet
                gw_info.ack_dsn = FRAME_GET_DSN(msg->data);
//                id = FRAME_GET_PAYLOAD_PALLET_ID(msg->data);
//                if(Is_ID_Active(&gw_conn_list.mapping, id)!=0)
//                {
//                	if(id ==Find_Next_ID(current_id))
//                	{
//                		gw_conn_list.conn_device[id].loss_count=0;
//                	}
//                	else
//                	{
//                		gw_conn_list.conn_device[id].loss_count++;
//                	}
//                	current = id;
//                }

                DataToSend.flag = 1;
                DataToSend.dsn = gw_info.ack_dsn;
                DataToSend.pallet_id = FRAME_GET_PAYLOAD_PALLET_ID(msg->data);
                memcpy((unsigned char*) (&DataToSend.temperature), FRAME_GET_Point_PAYLOAD_TMP(msg->data), 4);
                ResuBuf_Write((unsigned char*)&DataToSend, sizeof(WaitForUpload_Typedef));
            }
            Message_Reset(msg);
        }
    	break;
    }
#if 0
	case GW_CONN_SEND_PLT_DATA_ACK:
    {
    	send_len = RF_Manual_Send(Build_Ack, &gw_info.ack_dsn);
    	TX_INDICATE();
    	temp_t0 = ClockTime();
    	gw_info.state = GW_CONN_DATA_ACK_TX_DONE;
        break;
    }
	case GW_CONN_DATA_ACK_TX_DONE:
	{
		if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
		{
			TX_INDICATE();
			gw_info.state = GW_CONN_SUSPEND;
		}
		break;
	}
#endif
	case GW_CONN_SUSPEND:
    {
    	RF_SetTxRxOff();

    	if(gw_info.dsn %10==0)
    	{
    		GatewaySetupTrig = 1;
    	}
    	else
    	{
//			if(DataToSend.flag != 0)
//			{
//				ResuBuf_Write((unsigned char*)&DataToSend, sizeof(WaitForUpload_Typedef));
//				DataToSend.flag = 0;
//			}
			 if((unsigned int)(ClockTime() - gw_info.wakeup_tick) <= BIT(31))
				 gw_info.state = GW_CONN_SEND_GW_BCN;
    	}
    	 break;
    }

	default:
		break;
	}
}

_attribute_ram_code_ void Gateway_RxIrqHandler(void)
{
    //set next rx_buf
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);

    if ((rx_packet[0] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet)))
    {
        MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else
    {
        if (FRAME_IS_PALLET_DATA(rx_packet))
        {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_PALLET_DATA);
        }
        else if (FRAME_IS_SETUP_PALLET_REQ(rx_packet))
        {
        	if((FRAME_GET_DEST_ADDR(rx_packet)==gw_info.mac_addr)&&(FRAME_GET_DEST_ID(rx_packet)==gw_info.gw_id))
        	{
            	MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_SETUP_REQ);
        	}
        }
        else
        {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_INVALID_DATA);
        }
    }
}

_attribute_ram_code_ void Gateway_RxTimeoutHandler(void)
{
    if (GW_CONN_PLT_DATA_WAIT == gw_info.state)
    {
        MsgQueue_Push(&msg_queue, NULL, GW_MSG_TYPE_PALLET_DATA_TIMEOUT);
    }
    else
    {

    }
}
_attribute_ram_code_ void Gateway_TxDoneHandle(void)
{
	MsgQueue_Push(&msg_queue, NULL, MSG_TX_DONE);
}

 _attribute_ram_code_ void Run_Gateway_Setup_Statemachine(Msg_TypeDef *msg)
{
	switch (gw_info.state)
	{
		case GW_SETUP_IDLE:
		{
	    	if(gw_info.setup_dsn>=gw_setup_infor.setup_num)
	    	{
	        	if(gw_conn_list.num!=0)
	        	{
	        		GPIO_SetBit(LED_GREEN);
	        		GPIO_ResetBit(LED_RED);
	        	}
	        	else
	        	{
	        		GPIO_SetBit(LED_RED);
	        		GPIO_ResetBit(LED_GREEN);
	        	}
	        	gw_info.state = GW_CONN_SEND_GW_BCN;
	    	}
	    	else
	    	{
	            gw_info.state = GW_SETUP_SEND_GB;
	    	}
			break;
		}
		case GW_SETUP_SEND_GB:
		{
	        
	        send_len = RF_Manual_Send(Build_GatewaySetupBeacon, (void*)&gw_info);
			TX_INDICATE();
	        temp_t0 = ClockTime();
	        gw_info.t0 = temp_t0;
	        gw_info.wakeup_tick = gw_info.t0 + MASTER_PERIOD*TickPerUs;
	        gw_info.state = GW_SETUP_GB_TX_DONE_WAIT;
			break;
		}
		case GW_SETUP_GB_TX_DONE_WAIT:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				//temp_t0 = ClockTime();
				gw_info.state = GW_SETUP_PLT_REQ_WAIT;
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //switch to rx mode and wait for
			}
			break;
		}
		case GW_SETUP_PLT_REQ_WAIT:
		{
	    	if(ClockTime() - (gw_info.wakeup_tick ) <= BIT(31))
	    	{
	    		temp_t0 = ClockTime();
	    		gw_info.state = GW_SETUP_IDLE;
	    	}
	        if (msg)
	        {
	            if (msg->type == GW_MSG_TYPE_SETUP_REQ)
	            {
	            	RX_INDICATE();
	            	gw_setup_infor.plt_addr = FRAME_GET_SRC_ADDR(msg->data);
	            	gw_setup_infor.plt_id = Find_Dev(&gw_conn_list, gw_setup_infor.plt_addr);
	            	if(gw_setup_infor.plt_id == 0)
	            	{
	            		gw_setup_infor.plt_id = Malloc_ID(&gw_conn_list);
	            		if(gw_setup_infor.plt_id !=0 )
	            		{
	            			Add_ID_List(&gw_conn_list, gw_setup_infor.plt_id, gw_setup_infor.plt_addr);
	            		}
	            		else
	            		{
	            			//todo 设备容量已满
	            		}
	            	}
	            	else
	            	{
	            		//todo 此设备已分配ID
	            	}
	    	        send_len = RF_Manual_Send(Build_GatewaySetupRsp, (void*)&gw_info);
	    	        temp_t0 = ClockTime();
	    	        gw_info.state = GW_SETUP_RSP_TX_DONE;
	    	        TX_INDICATE();
	            }
	            Message_Reset(msg);
	        }
			break;
		}
		case GW_SETUP_RSP_TX_DONE:
		{
			if(ClockTimeExceed(temp_t0, Estimate_SendData_Time_Length(send_len)) || ((msg)&&(msg->type==MSG_TX_DONE)))
			{
				TX_INDICATE();
				gw_info.state = GW_SETUP_PLT_REQ_WAIT;
				RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //resume to rx mode and continue to receive PALLET_MSG_TYPE_SETUP_REQ
			}
			break;
		}
		case GW_SETUP_SUSPEND:
		{
			if((unsigned int)(ClockTime() - gw_info.wakeup_tick) <= BIT(31))
			{
				gw_info.state = GW_SETUP_IDLE;
			}
			break;
		}
		default :
			break;
	}
}

_attribute_ram_code_ void Gateway_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;

    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    //run state machine
    if(GatewaySetupTrig==1)
    {
    	gw_info.setup_dsn = 0;
    	GatewaySetupTrig = 0;
		GPIO_ResetBit(LED_GREEN);
		TOGGLE_TEST_PIN();
    	if(gw_conn_list.num==0)
    	{
			gw_setup_infor.setup_num = GW_SETUP_BCN_NUM;
			gw_info.state = GW_SETUP_IDLE;
    	}
    	else
    	{
    		gw_setup_infor.setup_num = 1;
    		gw_info.state = GW_SETUP_SUSPEND;
    		if((unsigned int)(ClockTime() - gw_info.wakeup_tick) >= BIT(31))
    		{
    			gw_info.wakeup_tick = gw_info.t0 + MASTER_PERIOD*TickPerUs;
    		}
    		else
    		{
    			gw_info.wakeup_tick = gw_info.t0 + 2*MASTER_PERIOD*TickPerUs;
    		}
    	}
    }
    if(IS_GW_WITHIN_SETUP_STATE(gw_info.state))
    {
		Run_Gateway_Setup_Statemachine(pMsg);
    }
    else if(IS_GW_WITHIN_ASSOCIATE_STATE(gw_info.state))
    {
        Run_Gateway_Statemachine(pMsg);
    }
}

