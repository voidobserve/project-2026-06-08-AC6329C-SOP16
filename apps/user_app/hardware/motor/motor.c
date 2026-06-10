#include "motor.h"

#include "typedef.h"

#include "motor_driver.h"

// ===============================================================
// 
// ===============================================================

volatile motor_t motor;

// USER_TO_DO 需要改成从 flash 中读出数据，再根据数据进行初始化
void motor_init(void)
{
	motor.mode = MOTOR_MODE_FORWARD_ROTATION;
	motor.speed_sec_per_round = 8;
	motor.phase = 0;
	motor.step = 0;
}

/**
 * @brief 根据传过来的速度值，设置电机转速
 *
 * @param sec_per_round 范围：0~255，函数内会限制最小为 4秒/圈
 */
void motor_set_speed_sec_per_round(u8 sec_per_round)
{
	// 根据原来的参数 8、13、18、21、26、35，分别对应8秒一圈，13秒一圈...
	// 电机最高转速约为 4.28 s 一圈，对应每步所需的时间间隔为 1046 us
	// 那么 8.56s 一圈，对应每步所需的时间间隔约为 1046 * 2 us
	// 假设 y 为 时间间隔，x 为一圈所需时间	
	// 根据公式 y1 - y2 = k (x1 - x2) 和 y = kx，得出 k 约为 244.39(当 x >= 4.28 时)

	// 8 s一圈，每步所需时间间隔约为 1955.12
	// 13 s一圈，每步所需时间间隔约为 3177.07
	// 18 s一圈，每步所需时间间隔约为 4399.02
	// 21 s一圈，每步所需时间间隔约为 5132.19
	// 26 s一圈，每步所需时间间隔约为 6354.14
	// 35 s一圈，每步所需时间间隔约为 8553.65

	/*
		由于驱动电机每步的时间间隔最少为1046us，而处理函数每125us才调用一次，
		那么至少调用 9（8.368，取整为9） 次处理函数才驱动一个电机步，
		这里要限制处理函数的调用周期，不能小于9次
	*/
	u32 speed_delay_us = 0;
	// 调用周期为125us的处理函数的调用次数
	u8 call_times = 0;

	// 限制最小的驱动电机步的时间间隔
	if (sec_per_round <= 4)
	{
		/*
			电机最高转速约为 4.28 s 一圈，对应每步所需的时间间隔为 1046 us
			处理函数每125us才调用一次，
			那么至少调用 9（8.368，取整为9） 次处理函数才驱动一个电机步
		*/
		speed_delay_us = (u32)125 * 9;
	}
	else
	{
		// 如果速度值大于最小的驱动电机步的时间间隔，则根据速度值计算电机步的时间间隔
		speed_delay_us = (u32)sec_per_round * 24439 / 100;
		call_times = speed_delay_us / 125;

		/*
			如果处理函数的调用周期小于电机步的时间间隔，则将处理函数的调用周期增加1，
			向时间间隔较大的对齐(取整)
		*/
		if ((u32)125 * call_times < speed_delay_us)
		{
			call_times++;
			speed_delay_us = (u32)125 * call_times;
		}
	}

	motor.speed_sec_per_round = sec_per_round;
	motor_24byj48_set_dest_step_delay_us(speed_delay_us);
}




void motor_set_mode(motor_mode_t mode)
{
	motor.mode = mode;
	motor.phase = 0;
	motor.step = 0;

	motor_24byj48_stop();
}



void motor_handle_125us(void)
{
	// static u16 step = 0;

	motor_24byj48_time_add();
	if (0 == motor_24byj48_time_expire())
	{
		return;
	}

	motor_24byj48_time_clear();

	switch (motor.mode)
	{
	case MOTOR_MODE_STOP:
		motor_24byj48_stop();
		break;
	case MOTOR_MODE_FORWARD_ROTATION: // 正转
		motor_24byj48_step(MOTOR_DIR_CW);
		break;
	case	MOTOR_MODE_REVERSE_ROTATION: // 反转
		motor_24byj48_step(MOTOR_DIR_CCW);
		break;
	case MOTOR_MODE_FORWARD_THEN_REVERSE_ROTATION:
		if (motor.phase == 0)
		{
			motor_24byj48_step(MOTOR_DIR_CW);

			motor.step++;
			if (motor.step >= 4096)
			{
				motor.step = 0;
				motor.phase = 1;
			}
		}
		else if (1 == motor.phase)
		{
			motor_24byj48_stop();
			motor.phase = 2;
		}
		else if (2 == motor.phase)
		{
			motor_24byj48_step(MOTOR_DIR_CCW);

			motor.step++;
			if (motor.step >= 4096)
			{
				motor.step = 0;
				motor.phase = 3;
			}
		}
		else if (3 == motor.phase)
		{
			motor_24byj48_stop();
			motor.phase = 0;
		}

		// pritnf("motor.step == %u\n", );
		break;
	case MOTOR_MODE_REVERSE_THEN_FORWARD_ROTATION:
		if (motor.phase == 0)
		{
			motor_24byj48_step(MOTOR_DIR_CCW);

			motor.step++;
			if (motor.step >= 4096)
			{
				motor.step = 0;
				motor.phase = 1;
			}
		}
		else if (1 == motor.phase)
		{
			motor_24byj48_stop();
			motor.phase = 2;
		}
		else if (2 == motor.phase)
		{
			motor_24byj48_step(MOTOR_DIR_CW);

			motor.step++;
			if (motor.step >= 4096)
			{
				motor.step = 0;
				motor.phase = 3;
			}
		}
		else if (3 == motor.phase)
		{
			motor_24byj48_stop();
			motor.phase = 0;
		}
		break;
	case MOTOR_MODE_SOUND_CONTROL:

		break;
	}



}