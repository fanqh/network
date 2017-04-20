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

//frame parse definitions
#define FRAME_GET_TIMESTAMP(p)             ( p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24) )
#define FRAME_GET_PERIOD_CNT(p)            ( p[21] | (p[22]<<8) | (p[23]<<16) | (p[24]<<24) )
#define FRAME_GET_PALLET_ID(p)             ( p[25] )
#define FRAME_IS_GATEWAY_BEACON(p)         ( p[20] == FRMAE_TYPE_GATEWAY_BEACON)
#define FRAME_IS_PALLET_DATA(p)            ( p[22] == FRMAE_TYPE_PALLET_DATA)
#define FRAME_IS_ACK_TYPE(p)               ( p[13] == 0x02)
#define FRAME_IS_NODE_DATA(p)              ( p[22] == FRMAE_TYPE_NODE_DATA)
#define FRAME_IS_PALLET_BEACON(p)          ( p[20] == FRMAE_TYPE_PALLET_BEACON)
#define FRAME_IS_LENGTH_OK(p)              (p[0] == p[12]+13)
#define FRAME_IS_CRC_OK(p)                 ((p[p[0]+3] & 0x51) == 0x10)


//frame pack functions
extern void Build_GatewayBeacon(unsigned char *pBuf, GWInfo_TypeDef *pInfo);
extern void Build_PalletData(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_PalletBeacon(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
extern void Build_NodeData(unsigned char *pBuf, NodeInfo_TypeDef *pInfo);
extern void Build_Ack(unsigned char *pBuf, unsigned char dsn);

#endif /*_FRAME_H_*/