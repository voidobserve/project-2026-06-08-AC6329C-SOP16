#include "led_driver.h"
#include "gpio.h"
#include "mcpwm.h"

volatile led_driver_t led_driver;

void led_driver_init(void)
{
	struct pwm_platform_data mcpwm_arg;
	gpio_set_direction(LED_DRIVER_RED_PIN, 0); // 输出模式
	gpio_direction_output(LED_DRIVER_RED_PIN, 0);

	gpio_set_direction(LED_DRIVER_GREEN_PIN, 0); // 输出模式
	gpio_direction_output(LED_DRIVER_GREEN_PIN, 0);

	gpio_set_direction(LED_DRIVER_BLUE_PIN, 0); // 输出模式
	gpio_direction_output(LED_DRIVER_BLUE_PIN, 0);

	// 上电时配置为输入上拉，判断当前灯具类型是混白灯还是纯白灯
	gpio_set_die(LED_DRIVER_WHITE_PIN, 1);
	gpio_set_direction(LED_DRIVER_WHITE_PIN, 1);  //输入模式
	gpio_set_pull_up(LED_DRIVER_WHITE_PIN, 1);  //上拉
 

	/*
		检测脚读取到高电平，作为混白色灯，读取到低电平，作为纯白色灯
	*/
	u8 white_light_det_cnt = 0; // 纯白色灯检测计数
	u8 mixed_white_light_det_cnt = 0; // 混白色灯检测计数
	// u8 i = 0;
	// for (i = 0; i < 20; i++)
	// {
	// 	if (gpio_read(LED_DRIVER_WHITE_PIN))
	// 	{
	// 		mixed_white_light_det_cnt++;
	// 	}
	// 	else
	// 	{
	// 		white_light_det_cnt++;
	// 	}
 
	// 	clr_wdt();
	// 	delay(1);
	// }

	if (mixed_white_light_det_cnt >= white_light_det_cnt)
	{
		led_driver.is_mixed_white_light = 1;
		// R
		mcpwm_arg.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
		mcpwm_arg.pwm_ch_num = LED_DRIVER_PWM_RED_CHANNEL;                        //通道号
		mcpwm_arg.frequency = 1000;                            //1KHz
		mcpwm_arg.duty = 0;                                    //上电输出0%占空比
		mcpwm_arg.h_pin = LED_DRIVER_RED_PIN;                         //任意引脚
		mcpwm_arg.l_pin = -1;                                  //任意引脚,不需要就填-1
		mcpwm_arg.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
		mcpwm_init(&mcpwm_arg);
		// G
		mcpwm_arg.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
		mcpwm_arg.pwm_ch_num = LED_DRIVER_PWM_GREEN_CHANNEL;                        //通道号
		mcpwm_arg.frequency = 1000;                            //1KHz
		mcpwm_arg.duty = 0;                                    //上电输出0%占空比
		mcpwm_arg.h_pin = LED_DRIVER_GREEN_PIN;                         //任意引脚
		mcpwm_arg.l_pin = -1;                                  //任意引脚,不需要就填-1
		mcpwm_arg.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
		mcpwm_init(&mcpwm_arg);
		// B
		mcpwm_arg.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
		mcpwm_arg.pwm_ch_num = LED_DRIVER_PWM_BLUE_CHANNEL;                        //通道号
		mcpwm_arg.frequency = 1000;                            //1KHz
		mcpwm_arg.duty = 0;                                    //上电输出0%占空比
		mcpwm_arg.h_pin = LED_DRIVER_BLUE_PIN;                         //任意引脚
		mcpwm_arg.l_pin = -1;                                  //任意引脚,不需要就填-1
		mcpwm_arg.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
		mcpwm_init(&mcpwm_arg);
	}
	else
	{
		led_driver.is_mixed_white_light = 0;

		mcpwm_arg.pwm_aligned_mode = pwm_edge_aligned;         // 边沿对齐
		mcpwm_arg.pwm_ch_num = LED_DRIVER_PWM_WHITE_CHANNEL;   // 通道号
		mcpwm_arg.frequency = 1000;                            // 1KHz
		mcpwm_arg.duty = 0;                                    // 上电输出0%占空比
		mcpwm_arg.h_pin = LED_DRIVER_WHITE_PIN; 			   // 任意引脚
		mcpwm_arg.l_pin = -1;                                  // 任意引脚,不需要就填-1
		mcpwm_arg.complementary_en = 0;                        // 两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
		mcpwm_init(&mcpwm_arg);
	}


	// 如果是纯白色灯:(原本是用一个单独的定时器来驱动输出pwm)
	// gpio_set_die(IO_PORTB_06, 1);
	// gpio_direction_output(IO_PORTB_06, 0);
	// extern void timer_pwm_init(JL_TIMER_TypeDef * JL_TIMERx, u32 pwm_io, u32 fre, u32 duty);
	// timer_pwm_init(JL_TIMER0, IO_PORTB_06, 1000, 0);  //调timer做pwm  通道号 R c
}


/**
 * @brief 根据RGB值，设置对应的pwm占空比值
 *
 */
void led_driver_set_rgb_pwm_val(u8 r, u8 g, u8 b)
{
	u32 red_pwm_duty;
	u32 green_pwm_duty;
	u32 blue_pwm_duty;

	if (0 == led_driver.is_mixed_white_light)
	{
		return;
	}

	red_pwm_duty = (u32)r * 10000 / 255;
	green_pwm_duty = (u32)b * 10000 / 255;
	blue_pwm_duty = (u32)g * 10000 / 255; 

	mcpwm_set_duty(LED_DRIVER_PWM_RED_CHANNEL, red_pwm_duty);  // R
	mcpwm_set_duty(LED_DRIVER_PWM_GREEN_CHANNEL, blue_pwm_duty);  // G   
	mcpwm_set_duty(LED_DRIVER_PWM_BLUE_CHANNEL, green_pwm_duty);  // B
}

/**
 * @brief 根据白色值，设置对应的pwm占空比值
 *
 */
void led_driver_set_white_pwm_val(u8 white_val)
{
	if (led_driver.is_mixed_white_light)
	{
		return;
	}

	u32 white_pwm_duty;
	white_pwm_duty = (u32)white_val * 10000 / 255; 
	mcpwm_set_duty(LED_DRIVER_PWM_WHITE_CHANNEL, white_pwm_duty);
}


