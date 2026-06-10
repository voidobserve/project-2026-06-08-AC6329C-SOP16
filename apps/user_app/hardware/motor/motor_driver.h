#ifndef __MOTOR_DRIVER_H__
#define __MOTOR_DRIVER_H__

#include "typedef.h"
#include "gpio.h"

/*
	- 24BYJ48 电机参数
	电机减速比：		1/64
	步距角：			5.625°/64（每64步对应5.625°）
	转过一圈所需的步：   360 / 5.625 * 64 == 4096

	- 计算转速
	1 rpm == 1圈 / 分钟
	电机是 4096步 / 圈，那么
	1 rpm转速对应的步 == 4096
	2 rpm转速对应的步 == 2 * 4096
	n rpm转速对于的步 == n * 4096
	n rpm下，每一步所需时间：
	== 1分钟 / 每分钟转速对应的步 == 1 / (n * 4096)
	转换成以 us 为单位：
	1 * 60 * 1000 * 1000 / (n * 4096)

	根据规格书中的"最大应答周波数：≥1000PPS"，每秒最多响应1000次脉冲，
	并且不能长时间工作在这个区间
	那么每分钟最多响应 1000 * 60 个脉冲，每分钟 1000 * 60 步
	对应的转速是：
	1000 * 60 / 4096 约为 14 rpm，根据机械结构等参数差异，平均在 10 ~ 18 rpm

	假设最高转速是14rpm，那么每步所需时间（根据 1 * 60 * 1000 * 1000 / (n * 4096)）：
	1 * 60 * 1000 * 1000 / (14 * 4096)
	约为 1046 us，可以每 1 ms左右再给电机一次脉冲变化

	- 如果要设置转速
	根据原来的参数 8、13、18、21、26、35，分别对应8秒一圈，13秒一圈...
	电机最高转速约为 4.28 s 一圈，对应每步所需的时间间隔为 1046 us
	那么 8.56s 一圈，对应每步所需的时间间隔约为 1046 * 2 us
	假设 y 为 时间间隔，x 为一圈所需时间
	根据公式 y1 - y2 = k (x1 - x2) 和 y = kx，得出 k 约为 244.39(当 x >= 4.28 时)

	8 s一圈，每步所需时间间隔约为 1955.12
	13 s一圈，每步所需时间间隔约为 3177.07
	18 s一圈，每步所需时间间隔约为 4399.02
	21 s一圈，每步所需时间间隔约为 5132.19
	26 s一圈，每步所需时间间隔约为 6354.14
	35 s一圈，每步所需时间间隔约为 8553.65
*/

// 24BYJ48电机引脚定义（需要根据实际硬件连接修改）
// 默认使用PA0-PA3，实际使用时需要根据硬件连接调整
#define MOTOR_PIN_1     IO_PORTB_05
#define MOTOR_PIN_2     IO_PORT_DM 
#define MOTOR_PIN_3     IO_PORTA_00
#define MOTOR_PIN_4     IO_PORTB_07

// // 电机参数定义
// #define STEPS_PER_REVOLUTION    2048    // 24BYJ48: 32步 × 64减速比 = 2048步/转
// #define DEGREES_PER_STEP        (360.0f / STEPS_PER_REVOLUTION)  // 每步对应的角度

// 控制模式枚举
typedef enum {
	MOTOR_MODE_FULL_STEP = 0,    // 全步模式 - 4步序列
	MOTOR_MODE_HALF_STEP,        // 半步模式 - 8步序列  
} motor_control_mode_t;

// 步进模式定义
typedef enum {
	MOTOR_STEP_MODE_FULL = 0,       // 全步模式 (4步序列)
	MOTOR_STEP_MODE_HALF,           // 半步模式 (8步序列)  
	// MOTOR_STEP_MODE_MICROSTEP       // 微步模式 (这里实际使用半步模式实现)
} motor_step_mode_t;

// 电机方向定义
typedef enum {
	MOTOR_DIR_CW = 0,               // 顺时针（clock wise）
	MOTOR_DIR_CCW                   // 逆时针（count clock wise）
} motor_direction_t;

// 电机控制结构体
typedef struct {
	// USER_TO_DO 可能可以改成 u8 类型
	u32 in1_pin;                     // IN1引脚
	u32 in2_pin;                     // IN2引脚  
	u32 in3_pin;                     // IN3引脚
	u32 in4_pin;                     // IN4引脚
	motor_step_mode_t step_mode;    // 步进模式
	u16 current_step; // 当前步数位置 (控制内部的步进，半步模式：0~7，全步模式：0~3)
	motor_direction_t dir; // 旋转方向

	u32 cur_step_delay_us; // 当前步进计时
	u32 dest_step_delay_us; // 目标步进延时
	// float current_angle;            // 当前角度位置 (0-360度)
} motor_24byj48_t;

extern volatile motor_24byj48_t motor_24byj48;

void motor_24byj48_time_add(void);
u8 motor_24byj48_time_expire(void);
void motor_24byj48_time_clear(void);


void motor_24byj48_init();
void motor_24byj48_set_step_mode(motor_step_mode_t mode);
void motor_24byj48_step(motor_direction_t direction);
void motor_24byj48_move_steps(motor_24byj48_t* motor, u32 steps, motor_direction_t direction);
void motor_24byj48_set_dest_step_delay_us(u32 delay_us);

// void motor_24byj48_move_angle(motor_24byj48_t* motor, float angle, motor_direction_t direction);
// void motor_24byj48_move_to_angle(motor_24byj48_t* motor, float target_angle);
void motor_24byj48_stop(void);
// s32 motor_24byj48_angle_to_steps(float angle);
// float motor_24byj48_steps_to_angle(s32 steps);

#endif