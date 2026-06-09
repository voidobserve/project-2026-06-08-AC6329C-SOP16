#include "motor_driver.h"
#include "system/includes.h"

// 24BYJ48电机使用示例

static motor_24byj48_t my_motor;

// 初始化电机
void motor_example_init(void)
{
    // 假设电机连接到PA0-PA3引脚
    // motor_24byj48_init(&my_motor, 0, IO_PORTA_00, IO_PORTA_01, IO_PORTA_02, IO_PORTA_03);
    
    // 设置为半步模式以获得最高精度（最小角度调节）
    motor_24byj48_set_step_mode(&my_motor, MOTOR_STEP_MODE_HALF);
    
    // 设置速度为10 RPM
    motor_24byj48_set_speed(&my_motor, 10);
}

// 示例1：旋转指定角度
void motor_example_rotate_angle(float angle_degrees)
{
    // 顺时针旋转指定角度
    motor_24byj48_move_angle(&my_motor, angle_degrees, MOTOR_DIR_CW);
}

// 示例2：移动到绝对位置
void motor_example_move_to_position(float target_angle)
{
    // 移动到目标角度（自动选择最短路径）
    motor_24byj48_move_to_angle(&my_motor, target_angle);
}

// 示例3：精确微调（最小角度调节）
void motor_example_fine_tune(void)
{
    // 切换到半步模式（如果还没切换）
    motor_24byj48_set_step_mode(&my_motor, MOTOR_STEP_MODE_HALF);
    
    // 最小可调节角度约为0.0879度 (360/4096)
    // 移动1个半步步进（最小单位）
    motor_24byj48_step(&my_motor, MOTOR_DIR_CW);
}

// 示例4：完整旋转测试
void motor_example_full_rotation_test(void)
{
    // 全步模式下完整旋转一周（2048步）
    motor_24byj48_set_step_mode(&my_motor, MOTOR_STEP_MODE_FULL);
    motor_24byj48_move_steps(&my_motor, 2048, MOTOR_DIR_CW);
    
    // 半步模式下完整旋转一周（4096步）
    motor_24byj48_set_step_mode(&my_motor, MOTOR_STEP_MODE_HALF);
    motor_24byj48_move_steps(&my_motor, 4096, MOTOR_DIR_CCW);
}

// 示例5：停止电机
void motor_example_stop(void)
{
    motor_24byj48_stop(&my_motor);
}