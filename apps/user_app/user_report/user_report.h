#ifndef __USER_REPORT_H__
#define __USER_REPORT_H__

#include "typedef.h"
#include "btstack_typedef.h" 

  

// 发送缓冲区中，一条指令最大的长度：
#define USER_REPORT_BUFF_MAX_LEN 30
// 发送缓冲区中，最大的指令数量：
#define USER_REPORT_BUFF_MAX_NUM 30
typedef struct
{
    // 发送缓冲区
    u8 report_buff[USER_REPORT_BUFF_MAX_NUM][USER_REPORT_BUFF_MAX_LEN];
    u16 report_buff_len[USER_REPORT_BUFF_MAX_NUM];
    u8 report_buff_head;
    u8 report_buff_tail;
    u8 report_buff_num;

    // 存放指令
    void (*buf_put)(u8 *buff, u16 len);
    // 指令处理函数
    void (*buf_handle)(void);
} user_report_t;

extern volatile user_report_t user_report;
void user_conn_handle_set(hci_con_handle_t handle);
void user_report_self_addr_set(u8 *addr);

#endif