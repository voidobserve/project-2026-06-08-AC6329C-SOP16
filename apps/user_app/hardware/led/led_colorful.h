#ifndef __LED_COLORFUL_H__
#define __LED_COLORFUL_H__

#include "typedef.h"

#define LED_COLORFUL_NUM 1

typedef struct{
	u8 is_open; // 灯的开关状态
	
	u8 mode;
	u8 sub_mode;

	u8 r_val;
	u8 g_val;
	u8 b_val;
	u8 w_val;

	u8 brightness; // 亮度值

	u16 speed;


	// u8 color_nums; // 颜色数量
	u8 neopixel_permutations; // 颜色排列（例如 NEO_RGB、NEO_RBG）


} led_colorful_t;
 

#endif