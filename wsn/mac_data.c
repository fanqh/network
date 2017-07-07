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

#if 1

void Init_DataBase(Conn_List_Typedef *pDB)
{
#if 1
	memset((void*)pDB, 0, sizeof(Conn_List_Typedef));
#else

	//pDB->num = 0;
	//memset(pDB->mapping,0, MAP_BYTES_MAX*4);
	//memset(pDB->conn_device,0, sizeof(DeviceEntry_Typedef));
#endif
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
			for(j=0; j<32; j++)
			{
				if((pDb->mapping[i]&(1<<j)) != 0)
				{
					count --;
					id = i*32 + j+1;
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
		for(j=0; j<32; j++)
		{
			if((pDb->mapping[i]&(1<<j)) == 0)
			{
				return i*32 + j+1;
			}
		}
	}
	return 0;
}

void Set_ID_Active(unsigned int *mapping, unsigned char id)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;
	mapping[i] |= (1<<j);
}

void Set_ID_Inactive(unsigned int *mapping, unsigned char id)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;
	mapping[i] &= ~(1<<j);
}
void Clear_ID_ALL_Inactive(unsigned int *mapping)
{
	unsigned char i;

	for(i=0; i<MAP_BYTES_MAX; i++)
	{
		mapping[i] = 0;
	}
}

void Add_ID_List(Conn_List_Typedef *pDb, unsigned char id, unsigned short addr)
{
	pDb->conn_device[id].addr = addr;
	pDb->conn_device[id].id = id;

	pDb->num ++;
	Set_ID_Active(pDb->mapping, id);
}



void Delete_ID_List(Conn_List_Typedef *pDb, unsigned char id)
{
	pDb->conn_device[id].id = 0;
	pDb->conn_device[id].addr = 0;

	//pDb->mapping[i] &= ~(1<<j);
	pDb->num --;

	Set_ID_Inactive(pDb->mapping,id);
}

unsigned char Is_ID_Active(unsigned int *mapping, unsigned char id)
{
	unsigned char i, j;

	i = (id - 1)/32;
	j = (id - 1)%32;

	return ((mapping[i]&(1<<j))?1:0);
}

unsigned char Find_Next_ID(Conn_List_Typedef *pDb, unsigned char id)
{
	unsigned char a,b,i, j;

	if(id !=0)
	{
		a = (id - 1)/32;
		b = (id - 1)%32 + 1;
	}
	else
	{
		a = 0;
		b = 0;
	}

	for(i = a; i<MAP_BYTES_MAX; i++)
	{
		for(j=b; j<32; j++)
		{
			if(pDb->mapping[i]&(1<<j) !=0)
				return i*32 + j+1;
		}
	}
	return 0;

}



#endif




