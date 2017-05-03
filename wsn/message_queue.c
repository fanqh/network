#include "../common.h"
#include "config.h"
#include "message_queue.h"

_attribute_ram_code_ int MsgQueue_Clean(MsgQueue_Typedef *p)
{
	p->read = p->write = 0;
	memset(p->msg, 0, sizeof(Msg_TypeDef)*MSG_BUF_LEN);
	return 1;
}

_attribute_ram_code_ static unsigned char MsgQueue_Next(unsigned char i)
{
	  if (i == MSG_BUF_LEN - 1)
	    return 0;

	  return ++i;
}

_attribute_ram_code_ int MsgQueue_Push(MsgQueue_Typedef *p, unsigned char *data, unsigned char type)
{
	if(MsgQueue_Next(p->write) != p->read)
	{
		p->msg[p->write].data = data;
		p->msg[p->write].type = type;
		p->write = MsgQueue_Next(p->write);
		//memcpy(p->msg[p->write].buff, &pallet_info, 17);

		return 1;
	}
	else
	{
		return 0;
	}
}

_attribute_ram_code_ Msg_TypeDef *MsgQueue_Pop(MsgQueue_Typedef *p)
{
    Msg_TypeDef *ret = NULL;


    if(p->read != p->write)
    {
    	ret = &(p->msg[p->read]);
    	p->read = MsgQueue_Next(p->read);
    	return ret;
    }
    else
    {
    	return ret;
    }
    return ret;
}

_attribute_ram_code_ static void reset_buf(unsigned char *p, int len)
{
    assert(p);

    while (len--) {
        *p++ = 0;
    } 
}

_attribute_ram_code_ void Message_Reset(Msg_TypeDef *msg)
{

#if 1
    assert(msg);

    //reset rx_packet
    if (msg->data) {
        reset_buf(msg->data, RX_BUF_LEN);
    }
    //reset msg
    //msg->data == NULL;
    //msg->type == MSG_TYPE_NONE;
#endif
}
