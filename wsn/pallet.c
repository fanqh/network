#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "pallet.h"
#include "../debug/debug_queue.h"

#define NODE_TABLE_MAX_LEN   6

static PalletInfo_TypeDef pallet_info;
static MsgQueue_Typedef msg_queue;

static NodeEntry_Typedef node_table[NODE_TABLE_MAX_LEN];
static ev_time_event_t *pallet_setup_timer = NULL;

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;


//#define DEBUG 1


unsigned char test_buff[64];

#if DEBUG

static int state_error = 0;
typedef struct
{
	unsigned char msg_type;
	unsigned char prestate;
	unsigned char curstate;

}debug_t;

unsigned char pre = 0;

debug_t debug;
Debug_Queue_Typedef DebugQ;

//int tet;
void debug_enqueue(unsigned char type)
{
	debug.msg_type = type;
	debug.prestate = pre;
	debug.curstate = pallet_info.state;
	DebugQueue_Push(&DebugQ, (unsigned char*)&debug, 3);
}

#endif

typedef struct
{
	unsigned short pallet_mac;
	unsigned char pallet_id;
	unsigned char rst;
}device_infor_t;

NodeDataWaitSend_Typdedef node_data[3];
device_infor_t device_infor;
void Pallet_Init(void)
{

	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(device_infor_t), (unsigned char*)&device_infor);
	if(device_infor.pallet_mac == 0xffff)
		device_infor.pallet_mac = PALLET_MAC_ADDR;


    memset(&pallet_info, 0, sizeof(PalletInfo_TypeDef));
    pallet_info.mac_addr = device_infor.pallet_mac;
    pallet_info.dsn = pallet_info.mac_addr & 0xff;
	//pallet_info.pNodeData = node_data;


    //set rx buffer
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);

#if 0
    //DEBUG_SHOW_PIN
    GPIO_SetGPIOEnable(DEBUG_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(DEBUG_SHOW_PIN);

    GPIO_SetGPIOEnable(SUSPEND_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(SUSPEND_SHOW_PIN);
#endif
#if DEBUG
    DebugQueue_Reset(&DebugQ);
#endif
}


unsigned int bug_time, ee, count[8];
_attribute_ram_code_ void Run_Pallet_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now, j;

#if DEBUG
    pre = pallet_info.state;
#endif
    if (PALLET_STATE_IDLE == pallet_info.state) {
        pallet_info.state = PALLET_STATE_GW_BCN_WAIT;
        RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on

        GPIO_WriteBit(DEBUG_PIN, !GPIO_ReadOutputBit(DEBUG_PIN));
    }
    else if (PALLET_STATE_GW_BCN_WAIT == pallet_info.state)
    {
#if 1
        if (msg && (msg->type == PALLET_MSG_TYPE_GW_BCN))
        { //receive a valid GB
            now = ClockTime();
            unsigned int timestamp = FRAME_GET_TIMESTAMP(msg->data);
            //check validity of timestamp
            if ((unsigned int)(now - timestamp) > TIMESTAMP_INVALID_THRESHOLD*TickPerUs)
            {
                //msg->type == MSG_TYPE_NONE;
                return;
            }
            pallet_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
            pallet_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);

            if ((pallet_info.period_cnt % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM))
            {
				unsigned char i;
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                RF_SetTxRxOff();
                RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL);
                RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
                Build_PalletData(tx_buf, &pallet_info, node_data);

				for(i=0; i<3; i++)
					node_data[i].updata = 0;
                //update state
                pallet_info.state = PALLET_STATE_GW_ACK_WAIT;
            }
            else
            {
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
                pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
            }

            Message_Reset(msg);
        }
#endif
    }
    else if (PALLET_STATE_GW_ACK_WAIT == pallet_info.state) {
        if (msg)
        {
            pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
            pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*pallet_info.pallet_id*TickPerUs;
            if (msg->type == PALLET_MSG_TYPE_GW_ACK)
            {

                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
//                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
//                pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*PALLET_ID*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_GW_ACK_TIMEOUT) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
//                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
//                pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*PALLET_ID*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_INVALID_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
//                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
//                pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*PALLET_ID*TickPerUs;
            }
            else
            {
            	//added by fanqh
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
//                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
//                pallet_info.wakeup_tick = pallet_info.t0 + MASTER_PERIOD*PALLET_ID*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (PALLET_STATE_SUSPEND_BEFORE_PB == pallet_info.state) {
        //turn off receiver and go to suspend
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
        // while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
        pallet_info.state = PALLET_STATE_SEND_PB;
    }
    else if (PALLET_STATE_SEND_PB == pallet_info.state) {
        now = ClockTime();
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //switch to auto mode
        RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
        Build_PalletBeacon(tx_buf, &pallet_info);
        pallet_info.state = PALLET_STATE_NODE_DATA_WAIT;
    }
    else if (PALLET_STATE_NODE_DATA_WAIT == pallet_info.state) {
        if (msg) {
            if (msg->type == PALLET_MSG_TYPE_ED_DATA)
            {
            	unsigned char node_id;
            	GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                //ToDo: process received data submitted by end device
                //save the dsn for subsequent ack
                pallet_info.ack_dsn = msg->data[15];
				node_id = FRAME_GET_SRC_NODE_ID(msg->data);

				node_data[(node_id-1)%NODE_NUM].updata = 1;
				node_data[(node_id-1)%NODE_NUM].temperature = FRAME_GET_NODE_PAYLOAD(msg->data);
                //pallet_info.state = PALLET_STATE_SEND_NODE_ACK;

		        pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
		        pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_ED_DATA_TIMEOUT) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
                pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_INVALID_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
                pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
            }
            else
            {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
                pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
            	//todo ��������յ��������ݣ������˳�rx mode
            	//state_error ++;
            }

            Message_Reset(msg);
        }
    }
    else if (PALLET_STATE_SEND_NODE_ACK == pallet_info.state) {
        //send ack
        Build_Ack(tx_buf, pallet_info.ack_dsn);
        RF_StartStx(tx_buf, ClockTime() + TX_ACK_WAIT*TickPerUs);
        WaitUs(WAIT_ACK_DONE); //wait for tx done

        pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
        pallet_info.wakeup_tick = pallet_info.t0 + (MASTER_PERIOD - DEV_RX_MARGIN)*TickPerUs;
    }
    else if (PALLET_STATE_SUSPEND_BEFORE_GB == pallet_info.state) {
        //turn off receiver and go to suspend
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
        //while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick-TickPerUs);
        pallet_info.state = PALLET_STATE_IDLE;
    }
#if DEBUG
    if(pre != pallet_info.state)
    {

    	if(msg!= NULL)
    		debug_enqueue(msg->type );
    	else
    		debug_enqueue(MSG_TYPE_NONE );
    }
#endif
}

void Pallet_MainLoop(void)
{
    Msg_TypeDef *pMsg = NULL;
    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    //run state machine
    Run_Pallet_Statemachine(pMsg);

    //collect sensor data
}

unsigned char pack[4][32];
_attribute_ram_code_ void Pallet_RxIrqHandler(void)
{
	int i;
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
    	if(i>=4)
    		i = 0;

    	memcpy(pack[i], rx_packet, 32);
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
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_SETUP_REQ);
            }
            else {
                MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
            }
        }
        //if it is coor BCN frame, do sync and report data if it is this device's opportunity and go to suspend otherwise
        else if (FRAME_IS_GATEWAY_BEACON(rx_packet)) {
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
    if (PALLET_STATE_NODE_DATA_WAIT == pallet_info.state) {
        MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_ED_DATA_TIMEOUT);
    }
    else if (PALLET_STATE_GW_ACK_WAIT == pallet_info.state) {
        MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_GW_ACK_TIMEOUT);
    }
    else if (PALLET_STATE_SETUP_GW_RSP_WAIT == pallet_info.state) {
        MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_SETUP_GW_RSP_TIMEOUT);
    }
    else {

    }
}
unsigned short dest_mac;
int Pallet_SetupTimer_Callback(void *data)
{
    pallet_info.state = PALLET_STATE_SETUP_IDLE_2;
    return -1;
}

_attribute_ram_code_ void Run_Pallet_Setup_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;
    int i = 0;

    if (PALLET_STATE_SETUP_IDLE == pallet_info.state) {

    	RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
        Build_PalletSetupBeacon(tx_buf, &pallet_info);
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        RF_TxPkt(tx_buf);
        WaitUs(1000); //wait for tx done
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        pallet_info.state = PALLET_STATE_SETUP_NODE_REQ_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //switch to rx mode and wait for
    }
    else if (PALLET_STATE_SETUP_NODE_REQ_WAIT == pallet_info.state)
    {
        if (msg)
        {
            if (msg->type == PALLET_MSG_TYPE_SETUP_REQ)
            {


            	dest_mac = FRAME_GET_DST_ADDR(msg->data);
            	if(dest_mac == pallet_info.mac_addr)
            	{
					GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
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
					WaitUs(1000); //wait for tx done
					GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
					RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //resume to rx mode and continue to receive PALLET_MSG_TYPE_SETUP_REQ
				}

				Message_Reset(msg);
            }
        }
    }
}

void Pallet_SetupLoop(void)
{
    Msg_TypeDef *pMsg = NULL;

    pallet_setup_timer = ev_on_timer(Pallet_SetupTimer_Callback, NULL, PALLET_SETUP_PERIOD);
    assert(pallet_setup_timer);

    while (PALLET_STATE_SETUP_IDLE_2 != pallet_info.state) {
        ev_process_timer();

        //pop a message from the message queue
        pMsg = MsgQueue_Pop(&msg_queue);
        //run state machine
        Run_Pallet_Setup_Statemachine(pMsg);
    }
}

_attribute_ram_code_  void Run_Pallet_Setup_Statemachine2(Msg_TypeDef *msg)
{
    unsigned int now;

    if (PALLET_STATE_SETUP_IDLE_2 == pallet_info.state) {
        pallet_info.state = PALLET_STATE_SETUP_GW_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //switch to rx mode and wait for gateway setup beacon
    }
    else if (PALLET_STATE_SETUP_GW_BCN_WAIT == pallet_info.state) {
        if (msg) {
            if (msg->type == PALLET_MSG_TYPE_SETUP_GW_BCN) {
                now = ClockTime();
                pallet_info.state = PALLET_STATE_SETUP_BACKOFF;
                pallet_info.retry_times = 0;
                pallet_info.wakeup_tick = now + ((Rand()+pallet_info.mac_addr) & BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs;
                pallet_info.gw_addr = FRAME_GET_SRC_ADDR(msg->data);
            }

            Message_Reset(msg);
        }
    }
    else if (PALLET_STATE_SETUP_BACKOFF == pallet_info.state) {
        //turn off receiver and go to suspend
        RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //turn off RX mode
        // while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
        pallet_info.state = PALLET_STATE_SETUP_REQ_SEND;
    }
    else if (PALLET_STATE_SETUP_REQ_SEND == pallet_info.state) {
        if (pallet_info.retry_times < RETRY_MAX) {
            now = ClockTime();
            RF_StartStxToRx(tx_buf, now + RF_TX_WAIT*TickPerUs, RX_WAIT);
            Build_PalletSetupReq(tx_buf, &pallet_info);
            GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
            //update state
            pallet_info.state = PALLET_STATE_SETUP_GW_RSP_WAIT;
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
    else if (PALLET_STATE_SETUP_GW_RSP_WAIT == pallet_info.state) {
        if (msg) {
            if (msg->type == PALLET_MSG_TYPE_SETUP_GW_RSP) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.pallet_id = FRAME_GET_PALLET_ID_FROM_GATEWAY_SETUP(msg->data);
                pallet_info.state = PALLET_STATE_IDLE;
            }
            else {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                now = ClockTime();
                pallet_info.state = PALLET_STATE_SETUP_BACKOFF;
                pallet_info.wakeup_tick = now + ((Rand()+pallet_info.mac_addr) & BACKOFF_MAX_NUM)*BACKOFF_UNIT*TickPerUs;
                pallet_info.retry_times++;
            }

            Message_Reset(msg);
        }
    }
}

void Pallet_SetupLoop2(void)
{
    Msg_TypeDef *pMsg = NULL;

    while (PALLET_STATE_IDLE != pallet_info.state) {
        //pop a message from the message queue
        pMsg = MsgQueue_Pop(&msg_queue);
        //run state machine
        Run_Pallet_Setup_Statemachine2(pMsg);
    }
}
