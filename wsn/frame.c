#include "../common.h"
#include "frame.h"
#include "config.h"

_attribute_ram_code_ unsigned char Build_GatewayBeacon(unsigned char *pBuf, void *arg)
{
    unsigned char *p;
    unsigned char len;
	GWInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (GWInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID 
    *p++ = 0xbb;
    *p++ = 0xff; //dest address
    *p++ = 0xff;

    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;

    *p++ = FRMAE_TYPE_GATEWAY_BEACON;
	*p++ = pInfo->gw_id;
///    *p++ = pInfo->period_cnt;
//    *p++ = (pInfo->period_cnt >> 8) & 0xff;
//    *p++ = (pInfo->period_cnt >> 16) & 0xff;
//    *p++ = (pInfo->period_cnt >> 24) & 0xff;
//    *p++ = pInfo->pallet_id;
//    *p++ = pInfo->pallet_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}
_attribute_ram_code_ unsigned char Build_GatewaySetupBeacon(unsigned char *pBuf, void *arg)
{
    unsigned char *p;
    unsigned char len;
	GWInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (GWInfo_TypeDef*)arg;
    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xff; //dest PANID
    *p++ = 0xff;

    *p++ = 0xff; //dest address
    *p++ = 0xff;

    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;

    *p++ = FRMAE_TYPE_SETUP_GW_BEACON;

    *p++ = pInfo->gw_id;

    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

	return len;
}

_attribute_ram_code_ unsigned char Build_GatewaySetupRsp(unsigned char *pBuf, void *arg)
{
    unsigned char *p;
    unsigned char len;
	GWInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (GWInfo_TypeDef*)arg;
    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->pSetup_info->plt_addr & 0xff; //dest address
    *p++ = pInfo->pSetup_info->plt_addr >> 8;

    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_GW_RSP;
    *p++ = pInfo->pSetup_info->plt_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_Ack(unsigned char *pBuf, void* arg)
{
	//GWInfo_TypeDef *pInfo;

	//pInfo = (GWInfo_TypeDef*)arg;
    pBuf[0] = 4;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = 5;
    pBuf[5] = 0x02;
    pBuf[6] = 0x00;
    pBuf[7] = *(unsigned char *)arg; //dsn

	return (pBuf[0]-1);
}
_attribute_ram_code_ unsigned char Build_PalletSetupBeacon(unsigned char *pBuf, void *arg)
{
     unsigned char *p;
     unsigned char len;
     PalletInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (PalletInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = 0xff; //dest address
    *p++ = 0xff;
    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_PALLET_BEACON;
	*p++ = pInfo->pallet_id;
	*p++ = pInfo->gw_sn;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_NodeSetupReq(unsigned char *pBuf,  void *arg)
{
    unsigned char *p;
    unsigned char len;
    NodeInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (NodeInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->p_nd_setup_infor->plt_mac & 0xff; //dest address
    *p++ = pInfo->p_nd_setup_infor->plt_mac >> 8;
    *p++ = pInfo->mac_addr & 0xff; //src address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_NODE_REQ;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_PalletBeacon(unsigned char *pBuf, void *arg)
{
     unsigned char *p;
     unsigned char len;
     PalletInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (PalletInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = 0xff; //dest address
    *p++ = 0xff;

    *p++ = pInfo->pallet_id & 0xff; //src address
    *p++ = 0;

    *p++ = FRMAE_TYPE_PALLET_BEACON;
    *p++ = pInfo->gw_sn;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_PalletData(unsigned char *pBuf, void *arg)
{
    unsigned char *p;
    unsigned char len;
    NodeDataWaitSend_Typdedef* pnode;
    PalletInfo_TypeDef *pInfo;

    pInfo = (PalletInfo_TypeDef*) arg;
    pnode = (NodeDataWaitSend_Typdedef*)pInfo->pData;
    p = &pBuf[5];
    //build the 802.15.4 mac data frame header
    *p++ = 0x61; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn

    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->gw_id; //dest address
    *p++ = 0;

    *p++ = pInfo->pallet_id; //src address
    *p++ = 0;

    *p++ = FRMAE_TYPE_PALLET_DATA;
    *p++ = pInfo->gw_sn; ///add one line

	*p++ = pnode[0].updata;
    *p++ = (pnode[0].temperature) & 0xff;
    *p++ = (pnode[0].temperature >> 8) & 0xff;
    *p++ = (pnode[0].temperature >> 16) & 0xff;
    *p++ = (pnode[0].temperature >> 24) & 0xff;

    *p++ = pnode[1].updata;
    *p++ = (pnode[1].temperature) & 0xff;
    *p++ = (pnode[1].temperature >> 8) & 0xff;
    *p++ = (pnode[1].temperature >> 16) & 0xff;
    *p++ = (pnode[1].temperature >> 24) & 0xff;

	*p++ = pnode[2].updata;
    *p++ = (pnode[2].temperature) & 0xff;
    *p++ = (pnode[2].temperature >> 8) & 0xff;
    *p++ = (pnode[2].temperature >> 16) & 0xff;
    *p++ = (pnode[2].temperature >> 24) & 0xff;

    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_NodeData(unsigned char *pBuf, void *arg)
{
     unsigned char *p;
     unsigned char len;
     NodeInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (NodeInfo_TypeDef*)arg;
    //build the 802.15.4 mac data frame header
    *p++ = 0x61; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn

    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->pallet_id; //dest address
    *p++ = 0;

    *p++ = pInfo->node_id; //src address
    *p++ = 0;

    *p++ = FRMAE_TYPE_NODE_DATA;
    *p++ = pInfo->plt_dsn;

    *p++ = pInfo->tmp & 0xff;
    *p++ = (pInfo->tmp >> 8) & 0xff;
    *p++ = (pInfo->tmp >> 16) & 0xff;
    *p++ = (pInfo->tmp >> 24) & 0xff;

    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}

_attribute_ram_code_ unsigned char Build_PalletSetupRsp(unsigned char *pBuf, void *arg)
{
     unsigned char *p;
     unsigned char len;
     PalletInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (PalletInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->p_gp_Setup_infor->node_addr & 0xff; //dest address
    *p++ = pInfo->p_gp_Setup_infor->node_addr  >> 8;

    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;

    *p++ = FRMAE_TYPE_SETUP_PALLET_RSP;
    *p++ = pInfo->p_gp_Setup_infor->node_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}



_attribute_ram_code_ unsigned char Build_PalletSetupReq(unsigned char *pBuf, void *arg)
{
     unsigned char *p;
     unsigned char len;
     PalletInfo_TypeDef *pInfo;

	 p = &pBuf[5];
	 pInfo = (PalletInfo_TypeDef*)arg;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    
    *p++ = pInfo->p_gp_Setup_infor->gw_id; //dest PANID
    *p++ = 0;
    *p++ = pInfo->p_gp_Setup_infor->gw_mac & 0xff; //dest address
    *p++ = pInfo->p_gp_Setup_infor->gw_mac >> 8;

    *p++ = pInfo->mac_addr & 0xff; //src address
    *p++ = pInfo->mac_addr >> 8;

    *p++ = FRMAE_TYPE_SETUP_PALLET_REQ;
    //*p++ = pInfo->node_table_len;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;

    return len;
}



