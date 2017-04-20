#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "gateway.h"

static GWInfo_TypeDef gw_info;
static MsgQueue_Typedef msg_queue;

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;

void Gateway_Init(void)
{
    memset(&gw_info, 0, sizeof(GWInfo_TypeDef));
    gw_info.gw_id = GW_ID;
    gw_info.dsn = GW_ID*3;

    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //frequency 2425
   
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);
}

_attribute_session_(".ram_code") void Run_Gateway_Statemachine(Msg_TypeDef *msg)
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
            if (msg->type == GW_MSG_TYPE_PALLET_DATA) {
                //ToDo: process received data submitted by pallet
                //save the dsn for subsequent ack
                gw_info.ack_dsn = msg->data[15];
                gw_info.state = GW_STATE_SEND_PALLET_ACK;
            }
            else if (msg->type == GW_MSG_TYPE_PALLET_DATA_TIMEOUT) {
                gw_info.state = GW_STATE_SUSPEND;
                gw_info.wakeup_tick = gw_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
            }
            else if (msg->type == GW_MSG_TYPE_INVALID_DATA) {
                //Garbage packet
                gw_info.state = GW_STATE_SUSPEND;
                gw_info.wakeup_tick = gw_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
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
        gw_info.wakeup_tick = gw_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - RF_TX_WAIT)*TickPerUs;
    }
    else if (GW_STATE_SUSPEND == gw_info.state) {
        //turn off receiver and go to suspend
        // while((unsigned int)(ClockTime() - gw_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, gw_info.wakeup_tick);
        gw_info.state = GW_STATE_SEND_GW_BCN;
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

_attribute_session_(".ram_code") void Gateway_RxIrqHandler(void)
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
        else {
            MsgQueue_Push(&msg_queue, rx_packet, GW_MSG_TYPE_INVALID_DATA);
        }
    }
}

_attribute_session_(".ram_code") void Gateway_RxTimeoutHandler(void)
{
    if (GW_STATE_PALLET_DATA_WAIT == gw_info.state) {
        MsgQueue_Push(&msg_queue, NULL, GW_MSG_TYPE_PALLET_DATA_TIMEOUT);
    }
    else {

    }
}