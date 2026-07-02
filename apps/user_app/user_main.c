#include "user_include.h"
#include "includes.h"

#include "motor_driver.h"
#include "led_driver.h"
#include "sound_control.h"
#include "save_flash.h"
#include "det_50hz.h"

#include "led_strand_effect.h"
#include "rf24g_app.h"
#include "user_report.h"

void user_report_handle_task(void* p);
void user_msg_handle_task(void* p);
void user_main(void* p);



void user_init(void)
{
	led_driver_init();
	motor_24byj48_init();
	motor_init();
	sound_control_init();

	// USER_TO_DO 还没有添加 220V的50Hz 信号作为ws2812动画的时钟源
#if !USER_SHIELD_220V_50HZ_DETECTION
	det_50hz_init();
#endif

	user_data_init();

	full_color_init();

	sys_s_hi_timer_add(NULL, user_10ms_isr, 10);


	task_create(user_report_handle_task, NULL, "usr_report_task");
	task_create(user_msg_handle_task, NULL, "msg_task");
	task_create(user_main, NULL, "usr_main");

	printf("user_init\n");
}


void user_report_handle_task(void* p)
{
	while (1)
	{
		user_report.buf_handle();
		/*
			notify 需要一段时间才能发送，
			如果直接一次性修改发送，会导致旧数据被覆盖
		*/
		os_time_dly(1);
	}
}

void user_msg_handle_task(void* p)
{
	int msg[32] = { 0 };

	while (1)
	{
#if 1 
		int ret = os_taskq_pend("msg_task", msg, 1);
		// printf("recv msg\n");
		// printf("ret %d\n", ret);
		if (OS_TASKQ != ret) // 类型不对
		{
			continue;
		}

		if (msg[0] != Q_USER) // 不是用户消息
		{
			continue;
		}

		// 打印接收到的消息
		// for (u8 i =0; i < ARRAY_SIZE(msg); i++)
		// {
		//     printf("msg [%u]: %d\n", (u16)i, msg[i]);
		// }

		switch (msg[1])
		{
		case MSG_USER_SAVE_INFO:
		{
			user_data_save_enable();
		}
		break;
		}
#endif
	} // while (1)
}

void user_main(void* p)
{
	while (1)
	{
		// printf("user_main\n");
		// printf("fc_effect.rgb.w == %u\n", (u16)fc_effect.rgb.w);

		user_data_save_handle();

		time_clock_handler();  // 闹钟 

		/****添加 处理函数 start**/
		check_mic_sound();      // 采集声音并计算平均值
		music_static_sound();   // 声控，七彩灯定色转换 

		// rf24g_long_timer();

		WS2812FX_service(); // 注意，实际 这里约 20ms 才调用一次动画


		rf24g_key_handle();

		// clr_wdt(); // 线程里只喂狗，不调用系统延时，还是会出现复位问题
		os_time_dly(1);
	}
}

void user_10ms_isr(void)
{
	user_data_save_delay_add();

	// USER_TO_DO 实际不用这个时钟源，而是用220V的50Hz信号作为时钟源
	run_tick_per_10ms();
}

// 在定时器中断内调用
void user_125us_isr(void)
{
	motor_handle_125us();

}

