#include "../common.h"
#include "../drivers.h"
#include "config.h"
#include "frame.h"
#include "message_queue.h"
#include "gateway.h"
#include "../interface/pc_interface.h"

#define PALLET_TABLE_MAX_LEN 3

static unsigned int temp_t0 = 0;
static GWInfo_TypeDef gw_info;
static MsgQueue_Typedef msg_queue;

static PalletEntry_Typedef pallet_table[PALLET_TABLE_MAX_LEN];

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;


extern volatile unsigned char GatewaySetupTrig;


typedef struct
{
	unsigned short gateway_mac;
	unsigned char rst;
	unsigned long master_period;
}gateway_infor_t;

typedef struct
{
	unsigned char flag;
	unsigned char dsn;
	unsigned char pallet_id;
	NodeDataWaitSend_Typdedef ndata[3];

}WaitForUpload_Typedef;

gateway_infor_t device_infor;
WaitForUpload_Typedef DataToSend;
extern volatile unsigned char Tx_Done_falg;


void Gateway_Init(void)
{

	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(gateway_infor_t), (unsigned char*)&device_infor);
	if(device_infor.gateway_mac == 0xffff)
		device_infor.gateway_mac = PALLET_MAC_ADDR;

    memset(&gw_info, 0, sizeof(GWInfo_TypeDef));
    gw_info.mac_addr = device_infor.gateway_mac;
    //gw_info.dsn = gw_info.mac_addr & 0xff;
    gw_info.state = GW_STATE_RF_OFF;
	gw_info.gw_id = Rand()%255;

    RF_SetTxRxOff();
    RF_Init(RF_OSC_12M, RF_MODE_ZIGBEE_250K);
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
#if PA_MODE
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_TX);
#else
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
#endif
}
unsigned char Wait_Tx_Done(unsigned int timeout)//unit : us
{
	unsigned int t;
#if PA_MODE
	Tx_Done_falg = 0;
	t = ClockTime();
	while(!Tx_Done_falg)
	{
		if(ClockTime() - (t + timeout*TickPerUs)<= BIT(31))
			return FAILURE;
	}
	return SUCCESS;
#else
	t = ClockTime();
	while(!RF_TxFinish())
	{
		if(ClockTime() - (t + timeout*TickPerUs)<= BIT(31))
			return FAILURE;
	}
	RF_TxFinishClearFlag();
	return SUCCESS;
#endif
}

_attribute_ram_code_ void Run_Gateway_Statemachine(Msg_TypeDef *msg)
{
    if (GW_STATE_SEND_GW_BCN == gw_info.state)
    {
    	gw_info.t0 = ClockTime() + RF_TX_WAIT*TickPerUs;

    	RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
    	Build_GatewayBeacon(tx_buf, &gw_info);
        RF_TxPkt(tx_buf);

        TX_INDICATE();
        TIME_INDICATE();
        Wait_Tx_Done(TX_DONE_TIMEOUT);
        TX_INDICATE();

        temp_t0 = ClockTime();
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL);
        gw_info.state = GW_STATE_PALLET_DATA_WAIT;
    }
    else if (GW_STATE_PALLET_DATA_WAIT == gw_info.state)
    {
    	if(ClockTimeExceed(temp_t0, 2000))
    	{
    		gw_info.state = GW_STATE_SUSPEND;
    		gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD - RF_TX_WAIT)*TickPerUs;
    	}
    	else if (msg)
    	{
            if (msg->type == GW_MSG_TYPE_PALLET_DATA)
            {
            	RX_INDICATE();
                //ToDo: process received data submitted by pallet
                gw_info.ack_dsn = FRAME_GET_DSN(msg->data);
                DataToSend.flag = 1;
                DataToSend.dsn = gw_info.ack_dsn;
                DataToSend.pallet_id = FRAME_GET_PAYLOAD_PALLET_ID(msg->data);
                gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD - RF_TX_WAIT)*TickPerUs;

                memcpy(DataToSend.ndata, FRAME_GET_Point_PAYLOAD_TMP(msg->data), sizeof(NodeDataWaitSend_Typdedef)*3);
                gw_info.state = GW_STATE_SEND_PALLET_ACK;
            }
            Message_Reset(msg);
        }
    }
    else if (GW_STATE_SEND_PALLET_ACK == gw_info.state)
    {
        //send ack
    	RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
    	Build_Ack(tx_buf, gw_info.ack_dsn);
        RF_TxPkt(tx_buf);
        TX_INDICATE();
        Wait_Tx_Done(TX_DONE_TIMEOUT);
        TX_INDICATE();
        gw_info.state = GW_STATE_SUSPEND;
        //gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
    }
    else if (GW_STATE_SUSPEND == gw_info.state)
    {

    	if(DataToSend.flag != 0)
    	{
    		ResuBuf_Write((unsigned char*)&DataToSend, sizeof(WaitForUpload_Typedef));
    		DataToSend.flag = 0;
    	}

    	//GPIO_WriteBit(DEBUG_PIN, Bit_RESET);
        //turn off receiver and go to suspend
//#if SUPEND
//        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, gw_info.wakeup_tick);
//        gw_info.state = GW_STATE_SEND_GW_BCN;
//#else
    	 RF_SetTxRxOff();
         while((unsigned int)(ClockTime() - gw_info.wakeup_tick) > BIT(30));
    	 gw_info.state = GW_STATE_SEND_GW_BCN;
//#endif
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
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_SETUP_REQ);
        }
        else
        {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_INVALID_DATA);
        }
    }
}

_attribute_ram_code_ void Gateway_RxTimeoutHandler(void)
{
    if (GW_STATE_PALLET_DATA_WAIT == gw_info.state) {
        MsgQueue_Push(&msg_queue, NULL, GW_MSG_TYPE_PALLET_DATA_TIMEOUT);
    }
    else {

    }
}

_attribute_ram_code_ void Run_Gateway_Setup_Statemachine(Msg_TypeDef *msg)
{
    if (GW_STATE_SETUP_IDLE == gw_info.state)
    {
    	if(gw_info.dsn>=GW_SETUP_BCN_NUM)
    	{
        	if(gw_info.pallet_table_len!=0)
        	{
        		GPIO_SetBit(LED_GREEN);
        		GPIO_ResetBit(LED_RED);
        	}
        	else
        	{
        		GPIO_SetBit(LED_RED);
        		GPIO_ResetBit(LED_GREEN);
        	}
        	gw_info.state = GW_STATE_SEND_GW_BCN;
    	}
    	else
    	{
            gw_info.state = GW_STATE_SETUP_SEND_GB;
    	}
    }
    else if(GW_STATE_SETUP_SEND_GB == gw_info.state)
    {
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
        Build_GatewaySetupBeacon(tx_buf, &gw_info);
        TIME_INDICATE();
        temp_t0 = ClockTime();
        RF_TxPkt(tx_buf);
        TX_INDICATE();
        Wait_Tx_Done(TX_DONE_TIMEOUT);
        TX_INDICATE();

        gw_info.state = GW_STATE_SETUP_PALLET_REQ_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //switch to rx mode and wait for
    }
    else if (GW_STATE_SETUP_PALLET_REQ_WAIT == gw_info.state)
    {
    	if(ClockTime() - (temp_t0+MASTER_PERIOD*TickPerUs ) <= BIT(31))
    	{
    		temp_t0 = ClockTime();
    		gw_info.state = GW_STATE_SETUP_IDLE;
    	}
#if 1
        if (msg)
        {
        	int i = 0;
            if (msg->type == GW_MSG_TYPE_SETUP_REQ)
            {
            	RX_INDICATE();
                //send gateway setup response
                RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
                gw_info.pallet_addr = FRAME_GET_SRC_ADDR(msg->data);
                //check whether the node has been added in to the node table  todo need to optimize
                for (i = 0; i < gw_info.pallet_table_len; i++) {
                    if (gw_info.pallet_addr == pallet_table[i].pallet_addr) {
                        gw_info.pallet_id = pallet_table[i].pallet_id;
                        break;
                    }
                }
                if (i == gw_info.pallet_table_len) {
                    gw_info.pallet_table_len++;
                    gw_info.pallet_id = gw_info.pallet_table_len;
                    //add the new node to node table
                    assert(gw_info.pallet_table_len <= PALLET_TABLE_MAX_LEN);
                    pallet_table[i].pallet_addr = gw_info.pallet_addr;
                    pallet_table[i].pallet_id = gw_info.pallet_id;
                    pallet_table[i].pallet_node_num = FRAME_GET_PALLET_NODE_NUM(msg->data);
                }
                Build_GatewaySetupRsp(tx_buf, &gw_info);
                RF_TxPkt(tx_buf);
                TX_INDICATE();
                Wait_Tx_Done(TX_DONE_TIMEOUT);
                TX_INDICATE();

                RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //resume to rx mode and continue to receive PALLET_MSG_TYPE_SETUP_REQ
            }
            Message_Reset(msg);
        }
        #endif
    }
}

void Gateway_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;

    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    //run state machine
    if(GatewaySetupTrig==1)
    {
    	GatewaySetupTrig = 0;

    	GPIO_ResetBit(LED_GREEN);
		gw_info.dsn = 0;
    	gw_info.state = GW_STATE_SETUP_IDLE;
    	//gateway_setup_timer = ev_on_timer(Gateway_SetupTimer_Callback, NULL, GP_SETUP_PERIOD-1000*TickPerUs);
    }
    if(IS_GW_WITHIN_SETUP_STATE(gw_info.state))
    {
		//ev_process_timer();
		Run_Gateway_Setup_Statemachine(pMsg);
    }
    else if(IS_GW_WITHIN_ASSOCIATE_STATE(gw_info.state))
    {
        Run_Gateway_Statemachine(pMsg);
    }
}

