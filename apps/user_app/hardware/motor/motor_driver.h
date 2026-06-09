#ifndef __MOTOR_DRIVER_H__
#define __MOTOR_DRIVER_H__

#include "typedef.h"
#include "gpio.h"

/*
	24BYJ48 电机参数
	电机减速比：		1/64
	步距角：			5.625°/64（每64步对应5.625°）
	转过一圈所需的步：   360 / 5.625 * 64 == 4096

	计算转速
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
*/
// 24BYJ48 电机参数定义
// #define MOTOR_24BYJ48_STEPS_PER_REVOLUTION      2048    // 32 步 × 64减速比 = 2048 步/转
// #define MOTOR_24BYJ48_STEPS_PER_REVOLUTION      4096    // 64 步 × 64减速比 = 4096 步/转
// #define MOTOR_24BYJ48_DEGREE_PER_STEP           (360.0f / MOTOR_24BYJ48_STEPS_PER_REVOLUTION)  // 每步角度
// #define MOTOR_24BYJ48_MIN_ANGLE                 MOTOR_24BYJ48_DEGREE_PER_STEP   // 最小可调节角度

// 24BYJ48电机引脚定义（需要根据实际硬件连接修改）
// 默认使用PA0-PA3，实际使用时需要根据硬件连接调整
#define MOTOR_PIN_1     IO_PORTB_05
#define MOTOR_PIN_2     IO_PORT_DM 
#define MOTOR_PIN_3     IO_PORTA_00
#define MOTOR_PIN_4     IO_PORTB_07

// 电机参数定义
#define STEPS_PER_REVOLUTION    2048    // 24BYJ48: 32步 × 64减速比 = 2048步/转
#define DEGREES_PER_STEP        (360.0f / STEPS_PER_REVOLUTION)  // 每步对应的角度

// 控制模式枚举
typedef enum {
	MOTOR_MODE_FULL_STEP = 0,    // 全步模式 - 4步序列
	MOTOR_MODE_HALF_STEP,        // 半步模式 - 8步序列  
} motor_control_mode_t;

// 步进模式定义
typedef enum {
	MOTOR_STEP_MODE_FULL = 0,       // 全步模式 (4步序列)
	MOTOR_STEP_MODE_HALF,           // 半步模式 (8步序列)  
	MOTOR_STEP_MODE_MICROSTEP       // 微步模式 (这里实际使用半步模式实现)
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

	motor_direction_t dir; // 
	u16 current_step;               // 当前步数位置


	float current_angle;            // 当前角度位置 (0-360度)


	u32 step_delay_ms;              // 步进延时(毫秒)
} motor_24byj48_t;

extern volatile motor_24byj48_t motor_24byj48;

// 函数声明
void motor_24byj48_init();
void motor_24byj48_set_step_mode(motor_24byj48_t* motor, motor_step_mode_t mode);
void motor_24byj48_set_speed(motor_24byj48_t* motor, u16 rpm);
u16 motor_24byj48_get_min_delay_us(void);
void motor_24byj48_step(motor_24byj48_t* motor, motor_direction_t direction);
void motor_24byj48_move_steps(motor_24byj48_t* motor, u32 steps, motor_direction_t direction);
// void motor_24byj48_move_steps(u32 steps, motor_direction_t direction);

// void motor_24byj48_move_angle(motor_24byj48_t* motor, float angle, motor_direction_t direction);
// void motor_24byj48_move_to_angle(motor_24byj48_t* motor, float target_angle);
void motor_24byj48_stop(motor_24byj48_t* motor);
// s32 motor_24byj48_angle_to_steps(float angle);
// float motor_24byj48_steps_to_angle(s32 steps);

#endif