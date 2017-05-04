#include "../common.h"
#include "frame.h"
#include "config.h"

_attribute_ram_code_ void Build_GatewayBeacon(unsigned char *pBuf, GWInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID 
    *p++ = 0xbb;
    *p++ = 0xff; //dest address
    *p++ = 0xff;

    *p++ = (unsigned char)(GW_ID&0xff); //src address
    *p++ = (unsigned char)(GW_ID>>8);

    *p++ = FRMAE_TYPE_GATEWAY_BEACON;
    *p++ = pInfo->period_cnt & 0xff;
    *p++ = (pInfo->period_cnt >> 8) & 0xff;
    *p++ = (pInfo->period_cnt >> 16) & 0xff;
    *p++ = (pInfo->period_cnt >> 24) & 0xff;
    *p++ = pInfo->pallet_id;
    *p++ = pInfo->pallet_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

_attribute_ram_code_ void Build_PalletData(unsigned char *pBuf, PalletInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

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
    *p++ = pInfo->period_cnt & 0xff;
    *p++ = (pInfo->period_cnt >> 8) & 0xff;
    *p++ = (pInfo->period_cnt >> 16) & 0xff;
    *p++ = (pInfo->period_cnt >> 24) & 0xff;
    *p++ = pInfo->gw_id;
    *p++ = pInfo->pallet_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

_attribute_ram_code_ void Build_PalletBeacon(unsigned char *pBuf, PalletInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

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
    *p++ = pInfo->period_cnt & 0xff;
    *p++ = (pInfo->period_cnt >> 8) & 0xff;
    *p++ = (pInfo->period_cnt >> 16) & 0xff;
    *p++ = (pInfo->period_cnt >> 24) & 0xff;
    *p++ = pInfo->pallet_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

_attribute_ram_code_ void Build_NodeData(unsigned char *pBuf, NodeInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

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
    *p++ = pInfo->period_cnt & 0xff;
    *p++ = (pInfo->period_cnt >> 8) & 0xff;
    *p++ = (pInfo->period_cnt >> 16) & 0xff;
    *p++ = (pInfo->period_cnt >> 24) & 0xff;
    *p++ = pInfo->pallet_id;
    *p++ = pInfo->node_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

_attribute_ram_code_ void Build_Ack(unsigned char *pBuf, unsigned char dsn)
{
    pBuf[0] = 4;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = 5;
    pBuf[5] = 0x02;
    pBuf[6] = 0x00;
    pBuf[7] = dsn; //dsn
}

_attribute_ram_code_ void Build_NodeSetupReq(unsigned char *pBuf, NodeInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl
    *p++ = 0x98;
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->pallet_id & 0xff; //dest address
    *p++ = pInfo->pallet_id >> 8;
    *p++ = pInfo->mac_addr & 0xff; //src address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_NODE_REQ;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

_attribute_ram_code_ void Build_PalletSetupBeacon(unsigned char *pBuf, PalletInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

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
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

void Build_PalletSetupRsp(unsigned char *pBuf, PalletInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->node_addr & 0xff; //dest address
    *p++ = pInfo->node_addr >> 8;
    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_PALLET_RSP;
    *p++ = pInfo->node_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

void Build_GatewaySetupBeacon(unsigned char *pBuf, GWInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

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
    *p++ = FRMAE_TYPE_SETUP_GW_BEACON;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

void Build_PalletSetupReq(unsigned char *pBuf, PalletInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->gw_addr & 0xff; //dest address
    *p++ = pInfo->gw_addr >> 8;
    *p++ = pInfo->mac_addr & 0xff; //src address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_PALLET_REQ;
    *p++ = pInfo->node_table_len;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}

void Build_GatewaySetupRsp(unsigned char *pBuf, GWInfo_TypeDef *pInfo)
{
    unsigned char *p = &pBuf[5];
    int len = 0;

    //build the 802.15.4 mac data frame header
    *p++ = 0x41; //frame ctrl low: data frame type, PAN ID compression, no ack req
    *p++ = 0x98; //frame ctrl hig: short dst addr and src addr
    *p++ = ++(pInfo->dsn); //dsn
    *p++ = 0xaa; //dest PANID
    *p++ = 0xbb;
    *p++ = pInfo->pallet_addr & 0xff; //dest address
    *p++ = pInfo->pallet_addr >> 8;
    *p++ = pInfo->mac_addr & 0xff; //source address
    *p++ = pInfo->mac_addr >> 8;
    *p++ = FRMAE_TYPE_SETUP_GW_RSP;
    *p++ = pInfo->pallet_id;
    len = p - (&pBuf[5]);
    pBuf[0] = len + 1;
    pBuf[1] = 0;
    pBuf[2] = 0;
    pBuf[3] = 0;
    pBuf[4] = len + 2;
}


