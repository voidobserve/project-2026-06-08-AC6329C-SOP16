#include "user_report_app.h"
#include "user_report.h"

/*
	USER_TO_DO 给app反馈数据的接口，可以优化一下
*/
// void report_msg(u8 msg_type, u8 msg_data)
// {
// 	// 根据数据类型，查表
// }

void user_report_dev_type(u8 dev_type)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x07;
	buf[len++] = 0x01;
	buf[len++] = 0x01;
	buf[len++] = dev_type;

	user_report.buf_put(buf, len);
}

void user_report_sound_control_mode(u8 mode)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x06;
	buf[len++] = 0x06;
	buf[len++] = mode;

	user_report.buf_put(buf, len);
}

// 设备的开关状态
void user_report_dev_on_off_sta(u8 on_off_sta)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x01;
	buf[len++] = 0x01;
	buf[len++] = on_off_sta;

	user_report.buf_put(buf, len);
}

// 灯光的亮度
void user_report_brightness(u8 brightness)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x04;
	buf[len++] = 0x03;
	buf[len++] = brightness;

	user_report.buf_put(buf, len);
}

void user_report_speed(u8 speed)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x04;
	buf[len++] = 0x04;
	buf[len++] = speed;

	user_report.buf_put(buf, len);
}

// 声控模式下的灵敏度
void user_report_sound_control_sensitive(u8 sensitive)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x2F;
	buf[len++] = 0x05;
	buf[len++] = sensitive;

	user_report.buf_put(buf, len);
}

void user_report_alarm_data(u8 alarm_index, ALARM_CLOCK alarm_data)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x05;
	buf[len++] = alarm_index;
	buf[len++] = alarm_data.hour;
	buf[len++] = alarm_data.minute;
	buf[len++] = alarm_data.on_off;
	buf[len++] = alarm_data.mode;

	user_report.buf_put(buf, len);
}

void user_report_motor_speed_sec_per_round(u8 sec_per_round)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x2F;
	buf[len++] = 0x07;
	buf[len++] = sec_per_round;

	user_report.buf_put(buf, len);
}

void user_report_motor_mode(motor_mode_t mode)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x2F;
	buf[len++] = 0x06;
	switch (mode)
	{
	case MOTOR_MODE_STOP:
		buf[len++] = 0x00;
		break;
	case MOTOR_MODE_FORWARD_ROTATION:
		buf[len++] = 0x02;
		break;
	case MOTOR_MODE_REVERSE_ROTATION:
		buf[len++] = 0x01;
		break;
	case MOTOR_MODE_FORWARD_THEN_REVERSE_ROTATION:
		buf[len++] = 0x03;
		break;
	case MOTOR_MODE_REVERSE_THEN_FORWARD_ROTATION:
		buf[len++] = 0x04;
		break;
	case MOTOR_MODE_SOUND_CONTROL:
		buf[len++] = 0x05;
		break;
	default:
		break;
	}

	user_report.buf_put(buf, len);
}

void user_report_sound_control_type(u8 type)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x06;
	buf[len++] = 0x07;
	buf[len++] = type;

	user_report.buf_put(buf, len);
}
 
// 发送同步结束指令
void user_report_syn_end(void)
{
	u8 buf[10];
	u8 len = 0;

	buf[len++] = 0x01;
	buf[len++] = 0x03;
	buf[len++] = 0x00;

	user_report.buf_put(buf, len);
}

