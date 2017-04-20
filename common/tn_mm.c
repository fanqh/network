/********************************************************************************************************
 * @file     tn_mm.c
 *
 * @brief    This file provides set of functions for memory management
 *
 * @author   jian.zhang@telink-semi.com
 * @date     Oct. 8, 2016
 *
 * @par      Copyright (c) 2016, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *           The information contained herein is confidential property of Telink 
 *           Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *           of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *           Co., Ltd. and the licensee or the terms described here-in. This heading 
 *           MUST NOT be removed from this file.
 *
 *           Licensees are granted free, non-transferable use of the information in this 
 *           file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/
#include "tn_mm.h"

void tn_mem_init(struct mem_desc *m)
{
    memset(m->count, 0, m->num);
    memset(m->mem, 0, m->size*m->num);
}

void *tn_mem_alloc(struct mem_desc *m)
{
    int i;

    for(i = 0; i < m->num; ++i) {
        if(m->count[i] == 0) {
            ++(m->count[i]);
            return (void *)((char *)m->mem + (i * m->size));
        }
    }

    return 0;
}

char tn_mem_free(struct mem_desc *m, void *ptr)
{
    int i;
    char *ptr2;

    ptr2 = (char *)m->mem;
    for (i = 0; i < m->num; ++i) {
        if (ptr2 == (char *)ptr) {
            if (m->count[i] > 0) {
                --(m->count[i]);
            }
            return m->count[i];
        }
        ptr2 += m->size;
    }
    return -1;
}

int tn_mem_inmemb(struct mem_desc *m, void *ptr)
{
    return (char *)ptr >= (char *)m->mem &&
           (char *)ptr < (char *)m->mem + (m->num * m->size);
}

int tn_mem_numfree(struct mem_desc *m)
{
    int i;
    int num_free = 0;

    for(i = 0; i < m->num; ++i) {
        if(m->count[i] == 0) {
            ++num_free;
        }
    }

    return num_free;
}
