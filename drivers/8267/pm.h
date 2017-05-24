/*
 * pm.h
 *
 *  Created on: 2017-5-24
 *      Author: Administrator
 */

#ifndef PM_H_
#define PM_H_

#include "bsp.h"
#include "../../common.h"

/**
 *  @brief  Define wake up source
 */
typedef enum {
    WAKEUP_SRC_PAD        = BIT(4),
    WAKEUP_SRC_DIG_GPIO   = BIT(5) | 0X0800,
    WAKEUP_SRC_DIG_USB    = BIT(5) | 0X0400,
    WAKEUP_SRC_DIG_QDEC   = BIT(5) | 0X1000,  //0x6e[4] of 8267 is qdec wakeup enbale bit
    WAKEUP_SRC_TIMER      = BIT(6),
    WAKEUP_SRC_COMP       = BIT(7),
} WakeupSrc_TypeDef;

/**
 *  @brief  Define wake up status
 */
typedef enum {
    WAKEUP_STATUS_COMP       = BIT(0),
    WAKEUP_STATUS_TIMER      = BIT(1),
    WAKEUP_STATUS_DIG        = BIT(2),
WAKEUP_STATUS_PAD        = BIT(3),
} WakeupStatus_TypeDef;

/**
 *  @brief  Define low power mode
 */
enum {
    SUSPEND_MODE    = 0,
    DEEPSLEEP_MODE  = 1,
};

extern int PM_LowPwrEnter (int deepsleep, int wakeup_src, u32 wakeup_tick);
void PM_WakeupInit(void);


#endif /* PM_H_ */
