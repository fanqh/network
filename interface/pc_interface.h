#ifndef PC_INTERFACE_H
#define PC_INTERFACE_H

/*********************************************************************
0x808004    0x808005    0x808006    0x808007    0x808008    0x808009
ParaAddr_L  ParaAddr_H  fifo_size   fifo_cnt    wptr        rptr
0x80800a    0x80800b    0x80800c    0x80800d    0x80800e    0x80800f
ResAddr_L   ResAddr_H   fifo_size   fifo_cnt    wptr        rptr


��RAM�д�����һ��������ڴ濨������д��fifo�Ƿֿ��ģ����뽫������Ϊ �����fifo��ÿ��fifo���ȹ̶���wptr rptr�ֱ�Ϊд�������Ͷ�ȡ������
д�����������λ��ά��������������ֵ��MCUά����������ring buff����
**********************************************************************/
#define    PARA_BUF_ADDR    0X808004
#define    PARA_BUF_SIZE    0X808006
#define    PARA_BUF_CNT     0X808007
#define    PARA_BUF_WPTR    0X808008
#define    PARA_BUF_RPTR    0X808009

#define    RESU_BUF_ADDR    0X80800a
#define    RESU_BUF_SIZE    0X80800c
#define    RESU_BUF_CNT     0X80800d
#define    RESU_BUF_WPTR    0X80800e
#define    RESU_BUF_RPTR    0X80800f

#define    commandBuffSize   12
#define    commandBuffCnt    3
#define    resultBuffSize    32
#define    resultBuffCnt     3

extern unsigned char commandBuff[commandBuffCnt][commandBuffSize];
extern unsigned char resultBuff[resultBuffCnt][resultBuffSize];

void ParaBuf_Init(unsigned short BufAddr, unsigned char BufSize, unsigned char BufCnt);
unsigned int ParaBuf_GetCmd(void);
unsigned int ParaBuf_Read(unsigned char *pDestBuf, unsigned char len);

void ResuBuf_Init(unsigned short BufAddr, unsigned char BufSize, unsigned char BufCnt);
unsigned int ResuBuf_Write(unsigned char *pSrcBuf, unsigned char len);

#endif
