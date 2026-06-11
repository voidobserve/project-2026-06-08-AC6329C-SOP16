#ifndef __RF24G_APP_H__
#define __RF24G_APP_H__

#include "board_ac632n_demo_cfg.h"
#include "typedef.h"

#if TCFG_RF24GKEY_ENABLE
void rf24_key_handle(struct sys_event *event);
#pragma pack (1)
typedef struct
{
    u8 pair[3];
    u8 flag;    //0:表示该数组没使用，0xAA：表示改数组已配对使用
}rf24g_pair_t;
#pragma pack ()
extern rf24g_pair_t rf24g_pair[];        //需要写flash



#endif

void rf24g_key_handle(void);

#endif
