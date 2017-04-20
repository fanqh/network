/********************************************************************************************************
 * @file     compiler.h
 *
 * @brief    This file define some compiler related macros
 *
 * @author   xiaodong.zong@telink-semi.com
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
#pragma once

#ifdef __GNUC__
#define _attribute_packed_		__attribute__((packed))
#define _attribute_aligned_(s)	__attribute__((aligned(s)))
#define _attribute_session_(s)	__attribute__((section(s)))
#define _attribute_ram_code_  	_attribute_session_(".ram_code")
#define _attribute_custom_code_  	_attribute_session_(".custom") volatile
#define _attribute_no_inline_   __attribute__((noinline)) 
// #define _inline_ 				extern __attribute__ ((gnu_inline)) inline
#else
#define _attribute_packed_
#define _attribute_aligned_(s)	__declspec(align(s))
#define _attribute_session_(s)
#define _attribute_ram_code_
#define _attribute_custom_code_
#define _attribute_no_inline_   __declspec(noinline) 
#endif
#define _inline_ 				inline				//   C99 meaning

