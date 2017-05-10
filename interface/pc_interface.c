#include "pc_interface.h"
#include "../drivers/8267/bsp.h"
#define    IO_BASE_ADDR    0x800000

unsigned char commandBuff[commandBuffCnt][commandBuffSize];
unsigned char resultBuff[resultBuffCnt][resultBuffSize];

void ParaBuf_Init(unsigned short BufAddr, unsigned char BufSize, unsigned char BufCnt)
{
	WRITE_REG16(PARA_BUF_ADDR, BufAddr);
	WRITE_REG8(PARA_BUF_SIZE, BufSize);
	WRITE_REG8(PARA_BUF_CNT, BufCnt);
	WRITE_REG8(PARA_BUF_WPTR, 0);
	WRITE_REG8(PARA_BUF_RPTR, 0);
}

unsigned int ParaBuf_GetCmd(void)
{
    unsigned char wptr = READ_REG8(PARA_BUF_WPTR);
    unsigned char rptr = READ_REG8(PARA_BUF_RPTR);
    if (wptr == rptr) {  //if buf is empty, return 0
        return 0;
    }
    else {
        unsigned short BufAddr = READ_REG16(PARA_BUF_ADDR);
        unsigned char BufSize = READ_REG8(PARA_BUF_SIZE);
        return READ_REG32(IO_BASE_ADDR+BufAddr+BufSize*rptr);     //mcu ����Ӧ����2λѰַ����������3λѰַ��������Ҫ��ȡ�ĵ�ַָ��
    }
}

unsigned int ParaBuf_Read(unsigned char *pDestBuf, unsigned char len)
{
    unsigned char rptr = READ_REG8(PARA_BUF_RPTR);
    unsigned short BufAddr = READ_REG16(PARA_BUF_ADDR);
    unsigned char BufSize = READ_REG8(PARA_BUF_SIZE);
    unsigned char BufCnt = READ_REG8(PARA_BUF_CNT);
    unsigned long ReadAddr = IO_BASE_ADDR + BufAddr + BufSize*rptr;

    int i = 0;
    for (i=0; i<len; i++) {
        pDestBuf[i] = READ_REG8(ReadAddr+i);
    }

    //adjust rptr
    WRITE_REG8(PARA_BUF_RPTR, (rptr+1)%BufCnt);

    return i;
}


void ResuBuf_Init(unsigned short BufAddr, unsigned char BufSize, unsigned char BufCnt)
{
	WRITE_REG16(RESU_BUF_ADDR, BufAddr);
    WRITE_REG8(RESU_BUF_SIZE, BufSize);
    WRITE_REG8(RESU_BUF_CNT, BufCnt);
    WRITE_REG8(RESU_BUF_WPTR, 0);
    WRITE_REG8(RESU_BUF_RPTR, 0);
}

unsigned long aaa;

unsigned char ss[4];

unsigned int ResuBuf_Write(unsigned char *pSrcBuf, unsigned char len)
{
    unsigned char wptr = READ_REG8(RESU_BUF_WPTR);
    unsigned char rptr = READ_REG8(RESU_BUF_RPTR);
    unsigned char BufCnt = READ_REG8(RESU_BUF_CNT);

    if (((wptr+1)%BufCnt) == rptr) { //if ResuBuf is full, do nothing and return 0
        return 0;
    }
    else {
        unsigned short BufAddr = READ_REG16(RESU_BUF_ADDR);
        unsigned char BufSize = READ_REG8(RESU_BUF_SIZE);
        unsigned long WriteAddr = IO_BASE_ADDR + BufAddr + BufSize*wptr;
        aaa =WriteAddr;


        if(aaa<=8)
        	aaa = 0;
        int i = 0;
	    for (i=0; i<len; i++) {
	    	WRITE_REG8(WriteAddr+i, pSrcBuf[i]);
	    	ss[i] = pSrcBuf[i];

	    }
        //adjust wptr
	    WRITE_REG8(RESU_BUF_WPTR, (wptr+1)%BufCnt);
        
        return i;   
    }
}
