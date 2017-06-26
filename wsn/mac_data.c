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
void Init_DataBase(Conn_List_Typedef *pDataBase)
{
	memset((void*)&pDataBase, 0, sizeof(Conn_List_Typedef));
}
/*
 * @return '0': have not mach addr '0-256' id of the match addr device
 *
 */
unsigned char Find_Dev(Conn_List_Typedef *pDb, unsigned short addr)
{
	unsigned int i, j;
	unsigned char id, count;

	if(pDb->num == 0)
		return 0;

	count = pDb->num;
	for(i=0; i<MAP_BYTES_MAX; i++)
	{
		if(pDb->mapping[i] != 0)
		{
			for(j=0; j<8; j++)
			{
				if((pDb->mapping[i]&(1<<j)) != 0)
				{
					count --;
					id = i*8 + j+1;
					if(pDb->conn_device[id].addr == addr)
						return id;
					if(count==0)
						return 0;
				}
			}
		}
	}
	return 0;
}
/*
 * @return '0' full, can't malloc free id '1-256' : id of malloc
 */
unsigned char Malloc_ID(Conn_List_Typedef *pDb)
{
	unsigned int i, j;

	for(i=0; i<MAP_BYTES_MAX; i++)
	{
		for(j=0; j<8; j++)
		{
			if((pDb->mapping[i]&(1<<j)) == 0)
			{
				return i*8 + j+1;
			}
		}
	}
	return 0;
}

void Add_ID_List(Conn_List_Typedef *pDb, unsigned id, unsigned short addr)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;

	pDb->conn_device[id].addr = addr;
	pDb->conn_device[id].id = id;

	pDb->mapping[i] |= (1<<j);
	pDb->num ++;
}

void Delete_ID_List(Conn_List_Typedef *pDb, unsigned id)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;

	pDb->conn_device[id].id = 0;
	pDb->conn_device[id].addr = 0;

	pDb->mapping[i] &= ~(1<<j);
	pDb->num --;
}

unsigned char Is_ID_Active(Conn_List_Typedef *pDb, unsigned id)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;

	return ((pDb->mapping[i]&(1<<j))?1:0);
}

#endif




