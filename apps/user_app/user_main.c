#include "user_include.h"
#include "includes.h"

#include "motor_driver.h"

void user_init(void)
{

}


void user_main(void)
{

}

// 在定时器中断内调用
void user_125us_isr(void)
{
	static volatile u8 cnt = 0;
	static volatile u8 dir = 0;

	cnt++;
	if (cnt < 8) // 125us * 8 == 1000 us 
	{
		return;
	}

	cnt = 0;

	motor_24byj48.step_delay_ms += 1;
	if (motor_24byj48.step_delay_ms >= (u32)2)
	{
		motor_24byj48.step_delay_ms = 0;

		if (dir == 0)
		{
			motor_24byj48_step(&motor_24byj48, MOTOR_DIR_CW);

			if (motor_24byj48.current_step >= 4096)
			{
				motor_24byj48.current_step = 0;
				dir = 1;
			}
		}
		else if (1 == dir)
		{
			motor_24byj48_stop(&motor_24byj48);
			dir = 2;
		}
		else if (2 == dir)
		{
			motor_24byj48_step(&motor_24byj48, MOTOR_DIR_CCW);

			if (motor_24byj48.current_step >= 4096)
			{
				motor_24byj48.current_step = 0;
				dir = 3;
			}
		}
		else if (3 == dir)
		{
			motor_24byj48_stop(&motor_24byj48);
			dir = 0;
		}
	}


	// if (0 == dir)
	// {
	// 	gpio_direction_output(IO_PORT_DM, 0);
	// 	dir = 1;
	// }
	// else
	// {
	// 	gpio_direction_output(IO_PORT_DM, 1);
	// 	dir = 0;
	// }
}

