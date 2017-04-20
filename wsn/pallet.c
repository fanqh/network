#include "../common.h"
#include "../drivers.h"

#include "config.h"
#include "frame.h"
#include "message_queue.h"

#include "pallet.h"

PalletInfo_TypeDef pallet_info;
static MsgQueue_Typedef msg_queue;

static unsigned char tx_buf[TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char rx_buf[RX_BUF_LEN*RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char rx_ptr = 0;

void Pallet_Init(void)
{
    memset(&pallet_info, 0, sizeof(PalletInfo_TypeDef));
    pallet_info.gw_id = GW_ID;
    pallet_info.pallet_id = PALLET_ID;
    pallet_info.dsn = PALLET_ID*10;

    //set rx buffer
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);
    //enable irq
    IRQ_EnableType(FLD_IRQ_ZB_RT_EN);
    IRQ_RfIrqEnable(FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT);
    IRQ_Enable();

    //config gpio showing timing
    GPIO_SetGPIOEnable(TIMING_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(TIMING_SHOW_PIN);

    //DEBUG_SHOW_PIN
    GPIO_SetGPIOEnable(DEBUG_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(DEBUG_SHOW_PIN);

    GPIO_SetGPIOEnable(SUSPEND_SHOW_PIN, Bit_SET);
    GPIO_ResetBit(SUSPEND_SHOW_PIN);
}

_attribute_session_(".ram_code") void Run_Pallet_Statemachine(Msg_TypeDef *msg)
{
    unsigned int now;

    if (PALLET_STATE_IDLE == pallet_info.state) {
        pallet_info.state = PALLET_STATE_GW_BCN_WAIT;
        RF_TrxStateSet(RF_MODE_RX, RF_CHANNEL); //turn Rx on
    }
    else if (PALLET_STATE_GW_BCN_WAIT == pallet_info.state) {
        if (msg && (msg->type == PALLET_MSG_TYPE_GW_BCN)) { //receive a valid GB
            now = ClockTime();

            unsigned int timestamp = FRAME_GET_TIMESTAMP(msg->data);
            //check validity of timestamp
            if ((unsigned int)(now - timestamp) > TIMESTAMP_INVALID_THRESHOLD*TickPerUs) {
                msg->type == MSG_TYPE_NONE;
                return;
            }
            pallet_info.t0 = timestamp - ZB_TIMESTAMP_OFFSET*TickPerUs;
            pallet_info.period_cnt = FRAME_GET_PERIOD_CNT(msg->data);
            if ((pallet_info.period_cnt % PALLET_NUM) == (pallet_info.pallet_id % PALLET_NUM)) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                RF_SetTxRxOff();
                RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); 
                RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
                Build_PalletData(tx_buf, &pallet_info);
                //update state
                pallet_info.state = PALLET_STATE_GW_ACK_WAIT;

               // GPIO_WriteBit(DEBUG_SHOW_PIN, !GPIO_ReadOutputBit(DEBUG_SHOW_PIN));
            }
            else {

                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
                pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*PALLET_ID*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (PALLET_STATE_GW_ACK_WAIT == pallet_info.state) {
        if (msg) {
            if (msg->type == PALLET_MSG_TYPE_GW_ACK) {

                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
                pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*PALLET_ID*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_GW_ACK_TIMEOUT) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));   
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
                pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*PALLET_ID*TickPerUs;
            }
            else if (msg->type == PALLET_MSG_TYPE_INVALID_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));   
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_PB;
                pallet_info.wakeup_tick = pallet_info.t0 + TIMESLOT_LENGTH*PALLET_ID*TickPerUs;
            }

            Message_Reset(msg);
        }
    }
    else if (PALLET_STATE_SUSPEND_BEFORE_PB == pallet_info.state) {
        //turn off receiver and go to suspend

    	GPIO_WriteBit(SUSPEND_SHOW_PIN, Bit_RESET);

        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
        // while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);
        pallet_info.state = PALLET_STATE_SEND_PB;

        GPIO_WriteBit(SUSPEND_SHOW_PIN, Bit_SET);
    }
    else if (PALLET_STATE_SEND_PB == pallet_info.state) {
        now = ClockTime();
        GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
        RF_SetTxRxOff();
        RF_TrxStateSet(RF_MODE_AUTO, RF_CHANNEL); //switch to auto mode
        RF_StartStxToRx(tx_buf , now + RF_TX_WAIT*TickPerUs, ACK_WAIT);
        Build_PalletBeacon(tx_buf, &pallet_info);
        pallet_info.state = PALLET_STATE_NODE_DATA_WAIT;

        GPIO_WriteBit(DEBUG_SHOW_PIN, Bit_SET);
    }
    else if (PALLET_STATE_NODE_DATA_WAIT == pallet_info.state) {
        if (msg) {
        		GPIO_WriteBit(DEBUG_SHOW_PIN, Bit_RESET);
            if (msg->type == PALLET_MSG_TYPE_ED_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                //ToDo: process received data submitted by end device
                //save the dsn for subsequent ack
                pallet_info.ack_dsn = msg->data[15];
                pallet_info.state = PALLET_STATE_SEND_NODE_ACK;


                //GPIO_WriteBit(DEBUG_SHOW_PIN, !GPIO_ReadOutputBit(DEBUG_SHOW_PIN));
            }
            else if (msg->type == PALLET_MSG_TYPE_ED_DATA_TIMEOUT) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));   
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
                pallet_info.wakeup_tick = pallet_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - DEV_RX_MARGIN)*TickPerUs;

                //GPIO_WriteBit(DEBUG_SHOW_PIN, !GPIO_ReadOutputBit(DEBUG_SHOW_PIN));
            }
            else if (msg->type == PALLET_MSG_TYPE_INVALID_DATA) {
                GPIO_WriteBit(TIMING_SHOW_PIN, !GPIO_ReadOutputBit(TIMING_SHOW_PIN));
                pallet_info.state = PALLET_STATE_SUSPEND_BEFORE_GB;
                pallet_info.wakeup_tick = pallet_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - DEV_RX_MARGIN)*TickPerUs;

                //GPIO_WriteBit(DEBUG_SHOW_PIN, !GPIO_ReadOutputBit(DEBUG_SHOW_PIN));
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
        pallet_info.wakeup_tick = pallet_info.t0 + (TIMESLOT_LENGTH*PALLET_NUM - DEV_RX_MARGIN)*TickPerUs;
    }
    else if (PALLET_STATE_SUSPEND_BEFORE_GB == pallet_info.state) {

    	GPIO_WriteBit(SUSPEND_SHOW_PIN, Bit_RESET);

        //turn off receiver and go to suspend
        RF_TrxStateSet(RF_MODE_TX, RF_CHANNEL); //turn off RX mode
        // while((unsigned int)(ClockTime() - pallet_info.wakeup_tick) > BIT(30));
        PM_LowPwrEnter(SUSPEND_MODE, WAKEUP_SRC_TIMER, pallet_info.wakeup_tick);

        GPIO_WriteBit(SUSPEND_SHOW_PIN, Bit_SET);
        pallet_info.state = PALLET_STATE_IDLE;
    }
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

_attribute_session_(".ram_code") void Pallet_RxIrqHandler(void)
{
    unsigned char *rx_packet = rx_buf + rx_ptr*RX_BUF_LEN;
    rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
    RF_RxBufferSet(rx_buf + rx_ptr*RX_BUF_LEN, RX_BUF_LEN, 0);

    if ((rx_packet[13] == 0) || (!FRAME_IS_CRC_OK(rx_packet)) || (!FRAME_IS_LENGTH_OK(rx_packet))) {
        // Garbage packet
        MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
    }
    //receive a valid packet
    else {
        //if it is coor ACK frame, check it and then go to suspend immediately
        if (FRAME_IS_ACK_TYPE(rx_packet) && (rx_packet[15] == pallet_info.dsn)) {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_GW_ACK);
        }
        //if it is coor BCN frame, do sync and report data if it is this device's opportunity and go to suspend otherwise
        else if (FRAME_IS_GATEWAY_BEACON(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_GW_BCN);
        }
        //if it is end device data frame, send ack immediately
        else if (FRAME_IS_NODE_DATA(rx_packet)) {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_ED_DATA);
        }
        else {
            MsgQueue_Push(&msg_queue, rx_packet, PALLET_MSG_TYPE_INVALID_DATA);
        }
    }   
}

_attribute_session_(".ram_code") void Pallet_RxTimeoutHandler(void)
{
    if (PALLET_STATE_NODE_DATA_WAIT == pallet_info.state) {
        MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_ED_DATA_TIMEOUT);
    }
    else if (PALLET_STATE_GW_ACK_WAIT == pallet_info.state) {
        MsgQueue_Push(&msg_queue, NULL, PALLET_MSG_TYPE_GW_ACK_TIMEOUT);
    }
    else {

    }
}
