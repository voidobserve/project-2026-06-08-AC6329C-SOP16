#ifndef __LED_DRIVER_H__
#define __LED_DRIVER_H__ 

#include "typedef.h"

// 配置R、G、B、W对应的引脚
#define LED_DRIVER_RED_PIN 			IO_PORTA_01
#define LED_DRIVER_GREEN_PIN 		IO_PORTA_02
#define LED_DRIVER_BLUE_PIN 		IO_PORTA_07
#define LED_DRIVER_WHITE_PIN 		IO_PORTB_06

#define LED_DRIVER_PWM_RED_CHANNEL 	pwm_ch0
#define LED_DRIVER_PWM_GREEN_CHANNEL pwm_ch1
#define LED_DRIVER_PWM_BLUE_CHANNEL pwm_ch2
#define LED_DRIVER_PWM_WHITE_CHANNEL pwm_ch3

typedef struct {
	u8 is_mixed_white_light; // 是否为混白色灯，0：否 ，1: 是
} led_driver_t;

extern volatile led_driver_t led_driver;

void led_driver_init(void);

void led_driver_set_rgb_pwm_val(u8 r, u8 g, u8 b);
void led_driver_set_white_pwm_val(u8 white_val);

#endif
