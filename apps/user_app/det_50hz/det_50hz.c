#include "det_50hz.h"
#include "mcpwm.h" // 将 pwm_ch3 复用作 220V 50Hz 信号检测脚的输入中断
#include "power_interface.h" // AT_VOLATILE_RAM_CODE

static volatile u32 tick = 0; // 用于记录 220V 50Hz 信号的周期计数，单位为毫秒

/**
 * @brief 根据 220V 50Hz 信号的检测脚的输入中断，更新 WS2812 动画的时基
 *
 */
AT_VOLATILE_RAM_CODE
void det_50hz_update_tick(void)
{
	if (gpio_read(IO_PORT_DP)) //  读取到高电平
	{
		tick += 20; // 50Hz信号，每个周期为20ms，检测到上升沿时，增加20ms
	}
}

AT_VOLATILE_RAM_CODE
void io_isr_cbfun_syn(u8 index)
{
	det_50hz_update_tick();
}

u32 det_50hz_get_tick(void)
{
	return tick;
}

// 初始化检测 220V 50Hz 信号的引脚
void det_50hz_init(void)
{
	gpio_set_pull_up(DET_50HZ_PIN, 0);  // 不上拉
	gpio_set_pull_down(DET_50HZ_PIN, 0);  // 不下拉
	gpio_set_die(DET_50HZ_PIN, 1);        // 过零触发检测口  普通io输入
	gpio_set_direction(DET_50HZ_PIN, 1);  // 输入模式

	set_io_ext_interrupt_cbfun(io_isr_cbfun_syn);
	io_ext_interrupt_init(pwm_ch3, DET_50HZ_PIN, 0); // 0：上升沿触发中断
}
