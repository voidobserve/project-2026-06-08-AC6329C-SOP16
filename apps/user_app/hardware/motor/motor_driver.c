#include "motor_driver.h"
#include "system/includes.h"
#include "gpio.h"

volatile motor_24byj48_t motor_24byj48;


// 步进序列定义
static const u8 full_step_sequence[4] = { 0x01, 0x02, 0x04, 0x08 };
/*
	0x01： 0001
	0x03： 0011
	0x02： 0010
	0x06： 0110
	0x04： 0100
	0x0C： 1100
	0x08： 1000
	0x09： 1001
*/
static const u8 half_step_sequence[8] = { 0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09 };
// static const u8 wave_drive_sequence[4] = { 0x01, 0x02, 0x04, 0x08 };

// // 当前电机状态
// static struct {
// 	motor_control_mode_t mode;
// 	u16 current_step;
// 	u32 total_steps;
// 	u32 step_delay_ms;      // 步进延迟时间（毫秒）
// 	bool is_running;
// 	bool direction;         // true: 正转, false: 反转
// } motor_state = { 0 };

// 简单的微秒级延时函数（基于固定系统时钟假设）
static void motor_delay_us(u32 us)
{
	if (us == 0) return;

	// 假设系统时钟为24MHz，每个循环大约需要4个时钟周期
	// 1us = 24个时钟周期，所以需要约6个循环
	volatile u32 delay_count = us * 6;

	while (delay_count > 0) {
		delay_count--;
		__asm__ volatile ("nop");
	}
}

// 设置电机引脚输出
static void motor_set_pins(u8 pattern)
{
	gpio_direction_output(MOTOR_PIN_1, (pattern & 0x01) ? 1 : 0);
	gpio_direction_output(MOTOR_PIN_2, (pattern & 0x02) ? 1 : 0);
	gpio_direction_output(MOTOR_PIN_3, (pattern & 0x04) ? 1 : 0);
	gpio_direction_output(MOTOR_PIN_4, (pattern & 0x08) ? 1 : 0);
}

// 获取当前步进序列长度
static u8 motor_get_sequence_length(motor_control_mode_t mode)
{
	switch (mode) {
	case MOTOR_MODE_HALF_STEP:
		return 8;
	case MOTOR_MODE_FULL_STEP:
	default:
		return 4;
	}
}

// 获取当前步进序列值
static u8 motor_get_sequence_value(motor_control_mode_t mode, u16 step_index)
{
	u8 seq_len = motor_get_sequence_length(mode);
	step_index = step_index % seq_len;

	switch (mode) {
	case MOTOR_MODE_FULL_STEP:
		return full_step_sequence[step_index];
	case MOTOR_MODE_HALF_STEP:
		return half_step_sequence[step_index];
	default:
		return 0;
	}
}

// 初始化电机驱动
void motor_24byj48_init(void)
{
	// 设置引脚
	motor_24byj48.in1_pin = IO_PORTB_05;
	motor_24byj48.in2_pin = IO_PORT_DM;
	motor_24byj48.in3_pin = IO_PORTA_00;
	motor_24byj48.in4_pin = IO_PORTB_07;
	motor_24byj48.step_mode = MOTOR_STEP_MODE_HALF;
	motor_24byj48.current_step = 0;
	motor_24byj48.current_angle = 0.0f;
	motor_24byj48.step_delay_ms = 2; // 默认 2ms 延迟

	// 初始化GPIO引脚为输出模式并设置初始值为0
	gpio_set_direction(motor_24byj48.in1_pin, 0); // 
	gpio_direction_output(motor_24byj48.in1_pin, 0);

	gpio_set_direction(motor_24byj48.in2_pin, 0); // 
	gpio_direction_output(motor_24byj48.in2_pin, 0);

	gpio_set_direction(motor_24byj48.in3_pin, 0); // 
	gpio_direction_output(motor_24byj48.in3_pin, 0);

	gpio_set_direction(motor_24byj48.in4_pin, 0); // 
	gpio_direction_output(motor_24byj48.in4_pin, 0);

	// 关闭所有线圈
	motor_set_pins(0x00);
}

// 设置电机步进模式
void motor_24byj48_set_step_mode(motor_24byj48_t* motor, motor_step_mode_t mode)
{
	motor->step_mode = mode;
	motor->current_step = 0;
}

// 设置电机速度（RPM）
void motor_24byj48_set_speed(motor_24byj48_t* motor, u16 rpm)
{
	// USER_TO_DO 待修改
	if (rpm == 0) rpm = 1; // 防止除零

	// 计算每步所需的时间（微秒）
	// 2048步/转，所以每分钟需要 2048 * rpm 步
	// 每步时间 = 60秒 / (2048 * rpm) = 60000000微秒 / (2048 * rpm)

	// u32 delay_us = 60000000UL / (2048UL * rpm);

	// // 限制延迟范围
	// if (delay_us < 500) delay_us = 500;    // 最小延迟500us（约58 RPM）
	// if (delay_us > 100000) delay_us = 100000; // 最大延迟100ms（约0.3 RPM）

	// motor->step_delay_ms = delay_us;
}

// 获取最小延迟时间（微秒）
u16 motor_24byj48_get_min_delay_us(void)
{
	return 500;
}

/**
 * @brief 执行单步操作
 *
 * @attention 该操作内部没有延时，需要等到延时到来再调用
 *
 */
void motor_24byj48_step(motor_24byj48_t* motor, motor_direction_t direction)
{
	u8 seq_len;
	u8 pattern;

	// 根据模式确定序列长度
	switch (motor->step_mode) {
	case MOTOR_STEP_MODE_HALF:
		seq_len = 8;
		break;
	case MOTOR_STEP_MODE_FULL:
	case MOTOR_STEP_MODE_MICROSTEP:
	default:
		seq_len = 4;
		break;
	}

	// 更新步数
	if (direction == MOTOR_DIR_CW) {
		motor->current_step = (motor->current_step + 1) % seq_len;
	}
	else {
		if (motor->current_step == 0) {
			motor->current_step = seq_len - 1;
		}
		else {
			motor->current_step--;
		}
	}

	// 获取当前步进模式下的序列值

	switch (motor->step_mode) {
	case MOTOR_STEP_MODE_HALF:
		pattern = half_step_sequence[motor->current_step];
		break;
	case MOTOR_STEP_MODE_FULL:
	default:
		pattern = full_step_sequence[motor->current_step % 4];
		break;
	}

	// 设置引脚输出
	gpio_direction_output(motor->in1_pin, (pattern & 0x01) ? 1 : 0);
	gpio_direction_output(motor->in2_pin, (pattern & 0x02) ? 1 : 0);
	gpio_direction_output(motor->in3_pin, (pattern & 0x04) ? 1 : 0);
	gpio_direction_output(motor->in4_pin, (pattern & 0x08) ? 1 : 0);

	// // 延时
	// motor_delay_us(motor->step_delay_us);

	// 更新角度位置
#if 0
	float angle_step = DEGREES_PER_STEP;
	if (motor->step_mode == MOTOR_STEP_MODE_HALF) {
		angle_step /= 2.0f; // 半步模式下角度减半
	}

	if (direction == MOTOR_DIR_CW) {
		motor->current_angle += angle_step;
	}
	else {
		motor->current_angle -= angle_step;
	}

	// 角度归一化到0-360度
	while (motor->current_angle >= 360.0f) {
		motor->current_angle -= 360.0f;
	}
	while (motor->current_angle < 0.0f) {
		motor->current_angle += 360.0f;
	}
#endif
}

// USER_TO_DO 
// 移动指定步数
// void motor_24byj48_move_steps(motor_24byj48_t *motor, s32 steps, motor_direction_t direction)
// {
//     if (steps == 0) return;

//     s32 abs_steps = (steps > 0) ? steps : -steps;
//     motor_direction_t actual_dir = (steps > 0) ? direction : 
//                                    (direction == MOTOR_DIR_CW ? MOTOR_DIR_CCW : MOTOR_DIR_CW);

//     for (s32 i = 0; i < abs_steps; i++) {
//         motor_24byj48_step(motor, actual_dir);
//     }
// }
void motor_24byj48_move_steps(motor_24byj48_t* motor, u32 steps, motor_direction_t direction)
{
	if (steps == 0) return;

	// u32 abs_steps = (steps > 0) ? steps : -steps;

	motor_direction_t actual_dir =
		(steps > 0) ? direction :
		(direction == MOTOR_DIR_CW ? MOTOR_DIR_CCW : MOTOR_DIR_CW);

	for (u32 i = 0; i < steps; i++) {
		motor_24byj48_step(motor, actual_dir);
	}
}

// 移动指定角度
// void motor_24byj48_move_angle(motor_24byj48_t* motor, float angle, motor_direction_t direction)
// {
// 	if (angle == 0.0f) return;

// 	// 计算需要的步数
// 	s32 steps = motor_24byj48_angle_to_steps(angle);
// 	// motor_24byj48_move_steps(motor, steps, direction);
// }

// // 移动到目标角度（绝对位置）
// void motor_24byj48_move_to_angle(motor_24byj48_t* motor, float target_angle)
// {
// 	// 规范化目标角度到0-360度
// 	while (target_angle >= 360.0f) {
// 		target_angle -= 360.0f;
// 	}
// 	while (target_angle < 0.0f) {
// 		target_angle += 360.0f;
// 	}

// 	// 计算最短路径
// 	float diff = target_angle - motor->current_angle;
// 	if (diff > 180.0f) {
// 		diff -= 360.0f;
// 	}
// 	else if (diff < -180.0f) {
// 		diff += 360.0f;
// 	}

// 	if (diff >= 0) {
// 		motor_24byj48_move_angle(motor, diff, MOTOR_DIR_CW);
// 	}
// 	else {
// 		motor_24byj48_move_angle(motor, -diff, MOTOR_DIR_CCW);
// 	}
// }

// 停止电机
void motor_24byj48_stop(motor_24byj48_t* motor)
{
	// 关闭所有线圈
	gpio_direction_output(motor->in1_pin, 0);
	gpio_direction_output(motor->in2_pin, 0);
	gpio_direction_output(motor->in3_pin, 0);
	gpio_direction_output(motor->in4_pin, 0);
}

// 角度转换为步数
// s32 motor_24byj48_angle_to_steps(float angle)
// {
//     // 处理负角度
//     if (angle < 0) {
//         angle = -angle;
//     }

//     // 计算步数
//     s32 steps = (s32)(angle / DEGREES_PER_STEP);
//     return steps;
// }

// // 步数转换为角度
// float motor_24byj48_steps_to_angle(s32 steps)
// {
//     return steps * DEGREES_PER_STEP;
// }