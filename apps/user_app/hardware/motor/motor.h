#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "typedef.h"

enum
{
	MOTOR_MODE_STOP, // 停止
	MOTOR_MODE_FORWARD_ROTATION, // 电机模式 正转
	MOTOR_MODE_REVERSE_ROTATION, // 电机模式 反转

	// 先正转，再反转
	MOTOR_MODE_FORWARD_THEN_REVERSE_ROTATION,
	// 先反转，再正转
	MOTOR_MODE_REVERSE_THEN_FORWARD_ROTATION,

	MOTOR_MODE_SOUND_CONTROL, // 电机模式 声控

};
typedef u8 motor_mode_t;

typedef struct
{
	motor_mode_t mode;
	u8 speed_sec_per_round; // 转速，单位：秒/圈
	u8 phase; // 在正反转期间使用
	u16 step; // 控制整个电机的步进 0 ~ 4095
} motor_t;

void motor_handle_125us(void);

#endif
