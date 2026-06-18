#include "user_report.h"
#include "ble_multi_profile.h" // 
#include "att.h"

void user_report_buf_put(u8* buff, u16 len);
void user_report_buf_handle(void);

static volatile hci_con_handle_t user_conn_handle;
static u8 ble_mac_addr[6];


volatile user_report_t user_report = {
	.report_buff_head = 0,
	.report_buff_tail = 0,
	.report_buff_num = 0,
	.buf_put = user_report_buf_put,
	.buf_handle = user_report_buf_handle,
};

void user_report_self_addr_set(u8* addr)
{
	memcpy(ble_mac_addr, addr, 6);
}

void user_conn_handle_set(hci_con_handle_t handle)
{
	user_conn_handle = handle;
}


void user_report_buf_put(u8* buff, u16 len)
{
	// 如果缓冲区满，下面的操作会覆盖缓冲区中旧的数据：

	// 先偏移索引，再存入数据
	user_report.report_buff_head++;
	if (user_report.report_buff_head >= USER_REPORT_BUFF_MAX_NUM)
	{
		user_report.report_buff_head = 0;
	}

	// 将指令保存到对应的缓冲区中
	memcpy(user_report.report_buff[user_report.report_buff_head], buff, len);
	// 将指令的长度保存到对应的缓冲区中
	user_report.report_buff_len[user_report.report_buff_head] = len;
	// 表示指令数量加1：
	user_report.report_buff_num++;
	if (user_report.report_buff_num >= USER_REPORT_BUFF_MAX_NUM)
	{
		user_report.report_buff_num = USER_REPORT_BUFF_MAX_NUM;
	}
}

// // 获取指令数量
// u8 ble_notify_param_get_num(void)
// {
//     return user_report.report_buff_num;
// }

void user_report_buf_handle(void)
{
	volatile u8 buff[USER_REPORT_BUFF_MAX_LEN + 6]; // 6字节为蓝牙mac地址

	if (user_report.report_buff_num == 0)
	{
		// 缓冲区中没有存放指令，直接返回
		return;
	}

	// 先偏移索引，再取出数据
	user_report.report_buff_tail++;
	if (user_report.report_buff_tail >= USER_REPORT_BUFF_MAX_NUM)
	{
		user_report.report_buff_tail = 0;
	}

#if 1 // 发送带有蓝牙mac地址的数据
	// 当前要发送的数据，由6个字节的蓝牙mac地址加上数据构成
	memcpy(buff, ble_mac_addr, 6);
	memcpy(
		buff + 6,
		user_report.report_buff[user_report.report_buff_tail],
		user_report.report_buff_len[user_report.report_buff_tail]);
	ble_comm_att_send_data(
		user_conn_handle,
		ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE,
		buff,	   //  
		user_report.report_buff_len[user_report.report_buff_tail] + 6, // 指令的长度
		ATT_OP_AUTO_READ_CCC);
		// ATT_OP_NOTIFY);
#endif 

#if 0 // 只发送不带有蓝牙mac地址的数据
	ble_comm_att_send_data(
		user_conn_handle,
		ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE,
		(user_report.report_buff[user_report.report_buff_tail]),	   // 指令
		user_report.report_buff_len[user_report.report_buff_tail], // 指令的长度
		ATT_OP_AUTO_READ_CCC);
#endif
	user_report.report_buff_num--;
}
