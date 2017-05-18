#include "../common.h"
#include "../drivers.h"


#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "gateway.h"
#include "../interface/pc_interface.h"

#define PALLET_TABLE_MAX_LEN 3

static GWInfo_TypeDef gw_info;
static MsgQueue_Typedef msg_queue;

static PalletEntry_Typedef pallet_table[PALLET_TABLE_MAX_LEN];
static ev_time_event_t *gateway_setup_timer = NULL;

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;


extern unsigned char SetupInitFlag;
extern volatile unsigned char GatewaySetupTrig;


typedef struct
{
	unsigned short gateway_mac;
	unsigned char rst;
	unsigned long master_period;
}gateway_infor_t;
gateway_infor_t device_infor;
void Gateway_Init(void)
{

	FLASH_PageRead(FLASH_DEVICE_INFOR_ADDR, sizeof(gateway_infor_t), (unsigned char*)&device_infor);
	if(device_infor.gateway_mac == 0xffff)
		device_infor.gateway_mac = PALLET_MAC_ADDR;

    memset(&gw_info, 0, sizeof(GWInfo_TypeDef));
    gw_info.mac_addr = device_infor.gateway_mac;
    gw_info.dsn = gw_info.mac_addr & 0xff;

    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //frequency 2425
   
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
#if PA_MODE
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_TX);
#else
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
#endif

    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
}


typedef struct
{
	unsigned char flag;
	unsigned char dsn;
	unsigned char pallet_id;
	NodeDataWaitSend_Typdedef ndata[3];

}WaitForUpload_Typedef;
WaitForUpload_Typedef DataToSend;

unsigned char tt[64];

_attribute_ram_code_ void Run_Gateway_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;

    if (GW_STATE_SEND_GW_BCN == gw_info.state) {
        now = ClockTime();
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        RF_StartStxToRx(tx_buf, now + RF_TX_WAIT*TickPerUs, RX_WAIT);
        
        //update coor info
        gw_info.t0 = now + RF_TX_WAIT*TickPerUs;
        gw_info.period_cnt++;
        gw_info.pallet_id++;
        Build_GatewayBeacon(tx_buf, &gw_info);
        gw_info.state = GW_STATE_PALLET_DATA_WAIT;
    }
    else if (GW_STATE_PALLET_DATA_WAIT == gw_info.state) {
        if (msg) {

        	gw_info.wakeup_tick = gw_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
            if (msg->type == GW_MSG_TYPE_PALLET_DATA) {
                //ToDo: process received data submitted by pallet
                //save the dsn for subsequent ack
                gw_info.ack_dsn = msg->data[15];

                memcpy(tt, msg->data, 64);
                DataToSend.flag = 1;
                DataToSend.dsn = gw_info.ack_dsn;
                DataToSend.pallet_id = FRAME_GET_PAYLOAD_PALLET_ID(msg->data);

                memcpy(DataToSend.ndata, FRAME_GET_Point_PAYLOAD_TMP(msg->data), sizeof(NodeDataWaitSend_Typdedef)*3);
                gw_info.state = GW_STATE_SEND_PALLET_ACK;
            }
            else if (msg->type == GW_MSG_TYPE_PALLET_DATA_TIMEOUT) {
                gw_info.state = GW_STATE_SUSPEND;
                //gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
            }
            else if (msg->type == GW_MSG_TYPE_INVALID_DATA) {
                //Garbage packet
                gw_info.state = GW_STATE_SUSPEND;
                //gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
            }
            else
            {
                gw_info.state = GW_STATE_SUSPEND;
                //gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (GW_STATE_SEND_PALLET_ACK == gw_info.state) {
        //send ack
        Build_Ack(tx_buf, gw_info.ack_dsn);
        RF_StartStx(tx_buf, ClockTime() + TX_ACK_WAIT*TickPerUs);
        WaitUs(WAIT_ACK_DONE); //wait for tx done

        gw_info.state = GW_STATE_SUSPEND;
        //gw_info.wakeup_tick = gw_info.t0 + (MASTER_PERIOD*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
    }
    else if (GW_STATE_SUSPEND == gw_info.state) {

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
         while((unsigned int)(ClockTime() - gw_info.wakeup_tick) > BIT(30));
    	 gw_info.state = GW_STATE_SEND_GW_BCN;
//#endif
    }
}

void Gateway_MainLoop(void)
{
    Msg_TypeDef* pMsg = NULL;
    //pop a message from the message queue
    pMsg = MsgQueue_Pop(&msg_queue);
    //run state machine
    Run_Gateway_Statemachine(pMsg);
}

_attribute_ram_code_ void Gateway_RxIrqHandler(void)
{
    //set next rx_buf
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);

    if ((rx_packet[0] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet))) {
        MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else {
        if (FRAME_IS_PALLET_DATA(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_PALLET_DATA);
        }
        else if (FRAME_IS_SETUP_PALLET_REQ(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_SETUP_REQ);
        }
        else {
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

int Gateway_SetupTimer_Callback(void *data)
{
	SetupInitFlag = 0;
	GatewaySetupTrig = 0;

	gw_info.state = GW_STATE_SEND_GW_BCN;
	RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //frequency 2425

    GPIO_SetBit(LED_PIN);
    WaitMs(1000);
    ev_unon_timer(&gateway_setup_timer);
    return -1;
}

_attribute_ram_code_ void Run_Gateway_Setup_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;
    int i = 0;

    if (GW_STATE_SETUP_IDLE == gw_info.state) {
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
        Build_GatewaySetupBeacon(tx_buf, &gw_info);
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        RF_TxPkt(tx_buf);
        WaitUs(1000); //wait for tx done
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        gw_info.state = GW_STATE_SETUP_PALLET_REQ_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //switch to rx mode and wait for         
    }
    else if (GW_STATE_SETUP_PALLET_REQ_WAIT == gw_info.state) {
        if (msg) {
            if (msg->type == GW_MSG_TYPE_SETUP_REQ) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                //send gateway setup response
                RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //switch to tx mode
                gw_info.pallet_addr = FRAME_GET_SRC_ADDR(msg->data);
                //check whether the node has been added in to the node table
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
                WaitUs(1000); //wait for tx done
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //resume to rx mode and continue to receive PALLET_MSG_TYPE_SETUP_REQ
            }

            Message_Reset(msg);
        }
    }
}

void Gateway_SetupLoop(void)
{
    Msg_TypeDef *pMsg = NULL;

    gateway_setup_timer = ev_on_timer(Gateway_SetupTimer_Callback, NULL, GW_SETUP_PERIOD);
    assert(gateway_setup_timer);

    while ((GW_STATE_SETUP_IDLE == gw_info.state) || (GW_STATE_SETUP_PALLET_REQ_WAIT == gw_info.state)) {
        ev_process_timer();

        //pop a message from the message queue
        pMsg = MsgQueue_Pop(&msg_queue);
        //run state machine
        Run_Gateway_Setup_Statemachine(pMsg);
    }
}
