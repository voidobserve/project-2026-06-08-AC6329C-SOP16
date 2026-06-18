#include "led_colorful_anim.h"
#include "WS2812FX.h"
#include "led_strand_effect.h" // 包含 fc_effect 定义

/**
 * @brief 七彩灯的呼吸动画
 */
u16 led_colorful_anim_breath(void)
{
    // static u32 last_sys_time = 0;
    // extern u32 sys_time_get(void);

	u8 brightness_max = fc_effect.b;
	u16 anim_speed = 0;
    // u16 anim_speed = fc_effect.dream_scene.speed;
    /*
        原本支持的动画速度：200 ~ 5000
        现在根据传递过来的速度对应的百分比值，映射到 200 ~ 5000
    */ 
    anim_speed = 5000 - (u32)fc_effect.report_speed * (5000 - 200) / 100;

    static u32 dest_color = BLACK; // 目标颜色
    /*
        每个步骤用时至少10ms，因为ws2812fx_service() 10ms调用一次

        从 0 到 511，
        步长为1，共512个步骤，至少 5120 ms 完成一次循环
        步长为2，共256个步骤，至少 2560 ms 完成一次循环

        那么速度值与循环的关系
        一次循环的时间 == 步骤 * 10ms
        一次循环的时间 == 512 / 步长 * 10ms
        速度值 == 512 / 步长 * 10ms
        步长 == 512 * 10ms / 速度值


        如果是从 0 到 指定亮度(brightness)
        步长为1，共 brightness + 1 步，至少 brightness * 10 ms 完成一次循环
        步长为2，共 (brightness + 1) / 2 步，至少 brightness * 10 ms / 2 完成一次循环

        速度值与亮度值的关系
        一次循环的时间 == (brightness + 1) / 步长 * 10ms
        速度值 == (brightness + 1) / 步长 * 10ms
        (brightness + 1) / 步长 == 速度值 / 10ms
        (brightness + 1) == 速度值 / 10ms * 步长
        步长 == (brightness + 1) * 10ms / 速度值
    */
    // u16 step = 0; // 步长
    // step = 512 * 10 / _seg->speed;

    static volatile u32 temp_step = 0;  // 累计放大了1000倍的步长，超过1000后，才执行动画的下一步骤
    static volatile u16 brightness = 0; // 亮度值
    u32 step = 0;                       // 步长（放大了1000倍）
    // step = ((u32)brightness_max + 1) * 10 * 1000 / anim_speed;

	// step = ((u32)brightness_max + 1) * 10 * 100 / anim_speed;

    step = ((u32)brightness_max + 1) * 10 * 1000 / anim_speed;
    // step = ((u32)brightness_max - 1) * 10 * 10000 / anim_speed; // （放大了10000倍）

    if (0 == _seg_rt->counter_mode_step &&
        0 == _seg_rt->aux_param &&
        0 == _seg_rt->counter_mode_call)
    {
        /*
            如果是第一次进入，设置默认颜色
            当前颜色为黑色，向目标颜色渐变（看起来像呼吸渐亮）
        */
        dest_color = _seg->colors[_seg_rt->aux_param];
        // dest_color = WS2812FX_color_blend(BLACK, _seg->colors[_seg_rt->aux_param], (u8)brightness_max);
        brightness = 0;
        temp_step = 0;
        // Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len);
    }

    temp_step += step;
    if (temp_step >= 1000)
    {
        // 有可能单次的步长会超过1000，这里用循环来逐个递减
        while (1)
        {
            if (temp_step < 1000)
            {
                break;
            }

            /*
                没有固定最大亮度的呼吸：
                brightness 变化范围： 0 -> brightness -> 0
            */
            _seg_rt->counter_mode_step++;
            if (temp_step >= 1000)
            {
                temp_step -= 1000;
            }
            else
            {
                temp_step = 0;
            }

            brightness = _seg_rt->counter_mode_step;
            if (brightness > (u16)brightness_max)
            {
                brightness = ((u16)brightness_max * 2) - brightness;
            }

            /*
                0 -> brightness_max，共 brightness_max 个步骤，灯光渐亮
                brightness_max -> 0，共 brightness_max 个步骤，灯光渐暗
            */
            if (_seg_rt->counter_mode_step >= ((u32)brightness_max * 2))
            {
                _seg_rt->counter_mode_step = 0;
                temp_step = 0;
                brightness = 0;

                _seg_rt->aux_param += 1; // 切换颜色数组 _seg->colors[] 中的下一个颜色
                if (_seg_rt->aux_param >= _seg->c_n)
                {
                    _seg_rt->aux_param = 0;
                }

                dest_color = _seg->colors[_seg_rt->aux_param];
                // Adafruit_NeoPixel_fill(BLACK, _seg->start, _seg_len); // 防止动画最后没有熄灭灯光
                // dest_color = WS2812FX_color_blend(BLACK, _seg->colors[_seg_rt->aux_param], (u8)brightness_max);

                // printf("__LINE__ %d\n", __LINE__);
                SET_CYCLE;
            }
        }
    }

    u32 color = WS2812FX_color_blend(BLACK, dest_color, (u8)brightness);
    Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

    // printf("brightness %u\n", (u16)brightness); //
    // printf("temp_step %lu\n", (u32)temp_step);  // 打印为0
    // printf("step %lu\n", (u32)step);
    // printf("_seg_rt->counter_mode_step %lu\n", (u32)_seg_rt->counter_mode_step);

    return 1; // ws2812fx_service() 10ms调用一次，这个值只需要小于10
}