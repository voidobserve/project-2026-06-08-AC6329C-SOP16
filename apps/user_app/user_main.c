#include "user_include.h"
#include "includes.h"

#include "motor_driver.h"
#include "led_driver.h"
#include "sound_control.h"
#include "save_flash.h"

#include "led_strand_effect.h"

void user_init(void)
{
	led_driver_init();
	motor_24byj48_init();
	motor_init();
	sound_control_init();

	user_data_init();

	full_color_init();

	sys_s_hi_timer_add(NULL, user_10ms_isr, 10);
	task_create(user_main, NULL, "usr_main");
}


void user_main(void)
{
	while (1)
	{
		// printf("user_main\n");

		user_data_save_handle();

		time_clock_handler();  //闹钟 

		/****添加 处理函数 start**/
		check_mic_sound();      // 采集声音并计算平均值
		music_static_sound();   // 声控，七彩灯定色转换

		// effect_stepmotor();    // 声控，电机的音乐效果 
		// stepmotor();            // 电机停止指令计时

		rf24g_long_timer();

		WS2812FX_service(); // 注意，这里约 20ms 才调用一次动画
		count_down_run();


		clr_wdt();
	}
}

void user_10ms_isr(void)
{
	user_data_save_delay_add();
	run_tick_per_10ms();
}

// 在定时器中断内调用
void user_125us_isr(void)
{
	motor_handle_125us();

}

