#ifndef __USER_INCLUDE_H__
#define __USER_INCLUDE_H__

#include "user_config.h"

enum
{
    MSG_NONE = 0x00, // 无效信息
 
    MSG_USER_SAVE_INFO, // 将数据写入flash
};

void user_init(void);

void user_125us_isr(void);
void user_10ms_isr(void);

#endif