#ifndef _FRAME_H_
#define _FRAME_H_

#include "gateway.h"
#include "pallet.h"
#include "node.h"

//frame type definitions
#define FRMAE_TYPE_GATEWAY_BEACON          0x01
#define FRMAE_TYPE_PALLET_DATA             0x02
#define FRMAE_TYPE_PALLET_BEACON           0x03
#define FRMAE_TYPE_NODE_DATA               0x04

#define FRMAE_TYPE_SETUP_PALLET_BEACON     0x05
#define FRMAE_TYPE_SETUP_NODE_REQ          0x06
#define FRMAE_TYPE_SETUP_PALLET_RSP        0x07

#define FRMAE_TYPE_SETUP_GW_BEACON         0x08
#define FRMAE_TYPE_SETUP_PALLET_REQ        0x09
#define FRMAE_TYPE_SETUP_GW_RSP            0x0a

//frame parse definitions  GateWay Beacon
#define FRAME_GET_TIMESTAMP(p)             ( p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24) )
#define FRAME_GET_PERIOD_CNT(p)            ( p[23] | (p[24]<<8) | (p[25]<<16) | (p[26]<<24) )
#define FRAME_GET_PALLET_ID(p)             ( p[27] )
#define FRAME_GET_DST_ADDR(p)              ( p[18] | (p[19]<<8) )
#define FRAME_GET_SRC_ADDR(p)              ( p[20] | (p[21]<<8) )
#define FRAME_GET_INCOME_DSN(p)            ( p[15] )


#define FRAME_GET_NODE_ID(p)               ( p[23] )
//#define FRAME_GET_PALLET_ID(p)             ( p[23] )

#define FRAME_GET_PALLET_NODE_NUM(p)       ( p[23] )

#define FRAME_IS_GATEWAY_BEACON(p)         (( p[22] == FRMAE_TYPE_GATEWAY_BEACON) )
#define FRAME_IS_PALLET_DATA(p)            ( p[22] == FRMAE_TYPE_PALLET_DATA)
#define FRAME_IS_ACK_TYPE(p)               ( p[13] == 0x02)
#define FRAME_IS_NODE_DATA(p)              ( p[22] == FRMAE_TYPE_NODE_DATA)
#define FRAME_IS_PALLET_BEACON(p)          ( p[22] == FRMAE_TYPE_PALLET_BEACON)
#define FRAME_IS_LENGTH_OK(p)              ( p[0] == p[12]+13)
#define FRAME_IS_CRC_OK(p)                 ((p[p[0]+3] & 0x51) == 0x10)

#define FRAME_IS_SETUP_PALLET_BEACON(p)    ( p[22] == FRMAE_TYPE_SETUP_PALLET_BEACON)
#define FRAME_IS_SETUP_NODE_REQ(p)         ( p[22] == FRMAE_TYPE_SETUP_NODE_REQ)
#define FRAME_IS_SETUP_PALLET_RSP(p)       ( p[22] == FRMAE_TYPE_SETUP_PALLET_RSP)
#define FRAME_IS_SETUP_GW_BEACON(p)        ( p[22] == FRMAE_TYPE_SETUP_GW_BEACON)
#define FRAME_IS_SETUP_PALLET_REQ(p)       ( p[22] == FRMAE_TYPE_SETUP_PALLET_REQ)
#define FRAME_IS_SETUP_GW_RSP(p)           ( p[22] == FRMAE_TYPE_SETUP_GW_RSP)

//frame pack functions
extern void Build_GatewayBeacon(unsigned char *pBuf, GWInfo_TypeDef *pInfo);
extern void Build_PalletData(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_PalletBeacon(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_NodeData(unsigned char *pBuf, NodeInfo_TypeDef *pInfo);
extern void Build_NodeSetupReq(unsigned char *pBuf, NodeInfo_TypeDef *pInfo);
extern void Build_Ack(unsigned char *pBuf, unsigned char dsn);

extern void Build_PalletSetupBeacon(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_PalletSetupRsp(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);

extern void Build_GatewaySetupBeacon(unsigned char *pBuf, GWInfo_TypeDef *pInfo);
extern void Build_PalletSetupReq(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_GatewaySetupRsp(unsigned char *pBuf, GWInfo_TypeDef *pInfo);


#endif /*_FRAME_H_*/