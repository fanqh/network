#ifndef _FRAME_H_
#define _FRAME_H_

#include "gateway.h"
#include "pallet.h"
#include "node.h"


typedef struct
{
	unsigned char updata;
	unsigned int temperature;
}NodeDataWaitSend_Typdedef;


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

//GateWay Beacon
#define FRAME_GET_TIMESTAMP(p)             ( p[8] | (p[9]<<8) | (p[10]<<16) | (p[11]<<24) )
#define FRAME_GET_PERIOD_CNT(p)            ( p[23] | (p[24]<<8) | (p[25]<<16) | (p[26]<<24) )
#define FRAME_GET_PALLET_ID(p)             ( p[27] )
#define FRAME_GET_DST_ADDR(p)              ( p[18] | (p[19]<<8) )
#define FRAME_GET_SRC_ADDR(p)              ( p[20] | (p[21]<<8) )
#define FRAME_GET_INCOME_DSN(p)            ( p[15] )

//node data
#define FRAME_GET_SRC_NODE_ID(p)            (p[20])
#define FRAME_GET_NODE_PAYLOAD(p)            (p[23] | p[24]<<8 | p[25]<<16  | p[26]<<24 )

// pallet data
#define FRAME_GET_PAYLOAD_PALLET_ID(p)            (p[20])
#define FRAME_GET_Point_PAYLOAD_TMP(p)            (p+23)

/*
 * pallet setup beacon
 */
#define FRAME_GET_GB_DSN(p)              			( p[15] )
#define FRAME_GET_DST_ADDR_NODE_REQ(p)              ( p[20] | (p[21]<<8) )
#define FRAME_PLT_SETUP_BCN_GET_SRC_MAC(p)			(p[20] | p[21]<<8)
#define FRAME_PLT_SETU_BCN_GET_PLT_ID(p)			(p[23])
#define FRAME_PLT_SETUP_RSP_DEST_ADDR(p)			( p[18] | (p[19]<<8) )
#define FRAME_PLT_SETUP_RSP_SRC_ADDR(p)				( p[20] | (p[21]<<8) )

//gateway setup
#define FRAME_GATEWAY_BCN_DSN(p)		(p[15])
#define FRAME_GATEWYA_SETUP_BCN_TOTAL_NUM(p)	((p[23]) | (p[24]<<8))
//gateway setup response
#define FRAME_GET_PALLET_ID_FROM_GATEWAY_SETUP(p)             ( p[23] )

#define FRAME_GET_NODE_ID(p)               ( p[23] )
//#define FRAME_GET_PALLET_ID(p)             ( p[23] )

#define FRAME_GET_PALLET_NODE_NUM(p)       ( p[23] )

#define FRAME_IS_GATEWAY_BEACON(p)         (( p[22] == FRMAE_TYPE_GATEWAY_BEACON)&&(p[16]==0xaa)&&(p[17]==0xbb) )
#define FRAME_IS_PALLET_DATA(p)            ( (p[22] == FRMAE_TYPE_PALLET_DATA)&&(p[16]==0xaa)&&(p[17]==0xbb))
#define FRAME_IS_ACK_TYPE(p)               ( p[13] == 0x02)
#define FRAME_IS_NODE_DATA(p)              ( (p[22] == FRMAE_TYPE_NODE_DATA)&&(p[16]==0xaa)&&(p[17]==0xbb))
#define FRAME_IS_PALLET_BEACON(p)          ( (p[22] == FRMAE_TYPE_PALLET_BEACON)&&(p[16]==0xaa)&&(p[17]==0xbb))
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
//extern void Build_PalletData(unsigned char *pBuf, PalletInfo_TypeDef *pInfo);
void Build_PalletData(unsigned char *pBuf, PalletInfo_TypeDef *pInfo, NodeDataWaitSend_Typdedef* pnode);

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
