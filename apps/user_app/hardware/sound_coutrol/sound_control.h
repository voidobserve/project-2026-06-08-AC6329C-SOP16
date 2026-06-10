#ifndef __SOUND_CONTROL_H__
#define __SOUND_CONTROL_H__ 

#include "typedef.h"

// 声控检测脚对应的adc通道
#define SOUND_CONTROL_DET_ADC_CHANNLE \
	AD_CH_PA8
// 声控信号检测脚：
#define SOUND_CONTROL_DET_PIN \
	IO_PORTA_08

void sound_control_init(void);

#endif