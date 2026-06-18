#ifndef __USER_REPORT_APP_H__
#define __USER_REPORT_APP_H__ 

#include "typedef.h"
#include "led_strand_effect.h" // ALARM_CLOCK 类型定义

void user_report_dev_type(u8 dev_type);
void user_report_dev_on_off_sta(u8 on_off_sta);

void user_report_brightness(u8 brightness);
void user_report_speed(u8 speed);

void user_report_sound_control_type(u8 type);
void user_report_sound_control_mode(u8 mode);
void user_report_sound_control_sensitive(u8 sensitive);

void user_report_alarm_data(u8 alarm_index, ALARM_CLOCK alarm_data);

void user_report_motor_mode(motor_mode_t mode);
void user_report_motor_speed_sec_per_round(u8 sec_per_round);

void user_report_syn_end(void);

#endif