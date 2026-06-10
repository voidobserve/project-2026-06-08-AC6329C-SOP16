#include "sound_control.h"
#include "gpio.h"
#include "adc_api.h"
void sound_control_init(void)
{
	adc_add_sample_ch(SOUND_CONTROL_DET_ADC_CHANNLE);  
	// 关闭上下拉
	gpio_set_pull_up(SOUND_CONTROL_DET_PIN, 0);
	gpio_set_pull_down(SOUND_CONTROL_DET_PIN, 0);
	gpio_set_die(SOUND_CONTROL_DET_PIN, 0);
	gpio_set_direction(SOUND_CONTROL_DET_PIN, 1);
}