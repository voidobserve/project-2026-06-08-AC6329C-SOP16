#ifndef __LED_COLORFUL_ANIM_H__
#define __LED_COLORFUL_ANIM_H__

#include "typedef.h"

// 七彩灯动画最大速度对应的数值
#define LED_COLORFUL_ANIM_MAX_SPEED_VAL ((u16)200)

// 七彩灯动画最小速度对应的数值
#define LED_COLORFUL_ANIM_MIN_SPEED_VAL ((u16)5000)

/*
	七彩灯动画速度百分比映射，
	将传递过来的速度百分比值映射到
	LED_COLORFUL_ANIM_MAX_SPEED_VAL ~ LED_COLORFUL_ANIM_MIN_SPEED_VAL
*/ 
#define LED_COLORFUL_ANIM_SPEED_VAL_MAP(speed_percent) \
	(LED_COLORFUL_ANIM_MIN_SPEED_VAL - \
	(u32)(speed_percent) * \
	(LED_COLORFUL_ANIM_MIN_SPEED_VAL - LED_COLORFUL_ANIM_MAX_SPEED_VAL) / 100)

u16  led_colorful_anim_jump(void);
u16 led_colorful_anim_gradual(void);
u16 led_colorful_anim_breath(void);
u16 led_colorful_anim_gradual_by_pause(void);
u16 led_colorful_anim_slide(void);
u16 led_colorful_anim_auto(void);

#endif