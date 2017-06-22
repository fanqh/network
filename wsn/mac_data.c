/*
 * mac_data.c
 *
 *  Created on: 2017-5-27
 *      Author: Administrator
 */
#include "config.h"
#include "frame.h"
#include "message_queue.h"
#include "mac_data.h"

#if 0
void* sys_memmove(void* dest, const void* src, size_t n)
{
    char*     d  = (char*) dest;
    const char*  s = (const char*) src;

    if (s>d)
    {
         // start at beginning of s
         while (n--)
            *d++ = *s++;
    }
    else if (s<d)
    {
        // start at end of s
        d = d+n-1;
        s = s+n-1;

        while (n--)
           *d-- = *s--;
    }
    return dest;
}
#define DEVICE_NUM	128

typedef struct
{
	unsigned short addr;
	unsigned char id;
	unsigned char rst;
}Conn_Device_Typedef;

typedef struct
{
	Conn_Device_Typedef Device_List[DEVICE_NUM];
	unsigned char num;
}DateBaseInfo_Typedef;

DateBaseInfo_Typedef database;

void Init_DataBase(DateBaseInfo_Typedef *pDataBase)
{
	memset((void*)&pDataBase, 0, sizeof(DateBaseInfo_Typedef));
}

unsigned char BinaryFind_Device(DateBaseInfo_Typedef *pDataBase, unsigned short addr)
{
	unsigned char low, high, mid;
	Conn_Device_Typedef device_info;

	if((pDataBase==NULL) || (pDataBase->num==0))
		return 0xff;

	low = 0;
	high = pDataBase->num - 1;

	while(low<=high)
	{
		mid = (low+high)/2;
		if(pDataBase->Device_List[mid].addr<addr)
		{
			low = mid + 1;
		}
		else if(pDataBase->Device_List[mid].addr>addr)
		{
			high = mid -1;
		}
		else
		{
			return mid;
		}
	}
	return 0xff;
}
unsigned char Insert_Device(DateBaseInfo_Typedef *pDataBase, Conn_Device_Typedef device)
{
}
unsigned char Delete_Device(DateBaseInfo_Typedef *pDataBase, Conn_Device_Typedef device)
{
}

unsigned char Find_Free_ID(void)
{
	static unsigned int Mapping_Index[(DEVICE_NUM + sizeof(unsigned int) - 1)/sizeof(unsigned int)];
	unsigned i,j;

	for(i=0; i< (DEVICE_NUM + sizeof(unsigned int) - 1)/sizeof(unsigned int); i++)
	{

	}
}
#endif




