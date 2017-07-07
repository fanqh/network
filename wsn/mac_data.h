/*
 * mac_data.h
 *
 *  Created on: 2017-5-27
 *      Author: Administrator
 */

#ifndef MAC_DATA_H_
#define MAC_DATA_H_

#include "../common.h"
#include "../drivers.h"

#define DEV_MAX_NUM		64
#define MAP_BYTES_MAX	(DEV_MAX_NUM+31)/32

typedef struct {
	unsigned char id;
	unsigned char loss_count;
    unsigned short addr;
} DeviceEntry_Typedef;

typedef struct{
	unsigned char rec_num;
	unsigned int mapping[MAP_BYTES_MAX];
}Rec_Infor_Typedef;

typedef struct {
	unsigned char num;
	unsigned int mapping[MAP_BYTES_MAX];
	DeviceEntry_Typedef conn_device[DEV_MAX_NUM];
}Conn_List_Typedef;


void Init_DataBase(Conn_List_Typedef *pDataBase);
unsigned char Malloc_ID(Conn_List_Typedef *pDb);
unsigned char Find_Dev(Conn_List_Typedef *pDb, unsigned short addr);
unsigned char Is_ID_Active(unsigned int *mapping, unsigned char id);
void Add_ID_List(Conn_List_Typedef *pDb, unsigned char id, unsigned short addr);
void Delete_ID_List(Conn_List_Typedef *pDb, unsigned char id);
void Set_ID_Active(unsigned int *mapping, unsigned char id);
void Set_ID_Inactive(unsigned int *mapping, unsigned char id);
void Clear_ID_ALL_Inactive(unsigned int *mapping);


#endif /* MAC_DATA_H_ */
