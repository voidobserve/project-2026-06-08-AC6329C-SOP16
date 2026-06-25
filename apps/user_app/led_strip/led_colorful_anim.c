#include "led_colorful_anim.h"
#include "WS2812FX.h"
#include "led_strand_effect.h" // 包含 fc_effect 定义
#include "ws2812fx_tool.h"
#include "Adafruit_NeoPixel.h" // 
#include <math.h>

// USER_TO_DO 
u16 led_colorful_anim_jump(void)
{
    u16 anim_speed = 0;
    // 根据传递过来的速度对应的百分比值，映射到 200 ~ 5000
    anim_speed = 5000 - (u32)fc_effect.report_speed * (5000 - 200) / 100;

    Adafruit_NeoPixel_fill(_seg->colors[_seg_rt->counter_mode_step], _seg->start, _seg_len);
    _seg_rt->counter_mode_step++;
    _seg_rt->counter_mode_step %= _seg->c_n;
    // if (_seg_rt->counter_mode_step == 0)
    // {
    //     // ws2811fx_set_cycle = 1;
    // }

    // return _seg->speed;
    return anim_speed;
}

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

// 七彩灯动画 渐变带停顿
u16 led_colorful_anim_gradual_by_pause(void)
{
    static uint8_t index = 0;
    static uint32_t c0, c1;
    uint32_t color;
    u16 anim_speed;
    int lum = _seg_rt->counter_mode_step;
    /*
        原本返回的速度值范围：10 ~ 500 ，
        现在根据传递过来的速度对应的百分比值，映射到 10 ~ 500 ，
    */
    anim_speed = (500 - (u32)fc_effect.report_speed * (500 - 10) / 100);

    if (lum > 255)
    {
        // lum = 0 -> 255 -> 0
        lum = 511 - lum;
    }

    if (_seg_rt->aux_param == 0)
    {
        // 如果刚进入该函数，初始化参数
        _seg_rt->aux_param = 1;
        index = 0;
        c1 = _seg->colors[index];
        index++;
        c0 = _seg->colors[index];
    }

    // 刚进入该函数时， _seg->colors[1]:目标颜色
    color = WS2812FX_color_blend(c1, c0, lum);

    Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

    if (_seg_rt->counter_mode_step == 256)
    {
        index++;
        index %= _seg->c_n;

        c1 = _seg->colors[index];

        _seg_rt->counter_mode_step++;
        // 变化完成一个颜色后，延时一段时间，再开始变换下一个颜色
        printf("next color\n");
        return (anim_speed * 100);
    }

    _seg_rt->counter_mode_step++;
    if (_seg_rt->counter_mode_step > 511)
    {
        _seg_rt->counter_mode_step = 0;
        index++;
        index %= _seg->c_n;
        c0 = _seg->colors[index];
        // SET_CYCLE; 
    }

    // return (_seg->speed / 5);
    return (anim_speed);
}


// 七彩灯的七色滑翔动画
u16 led_colorful_anim_slide(void)
{
    u16 anim_speed = 0;

    /*
        使用正弦插值在两种颜色之间平滑过渡，形成滑翔效果。
        每次过渡耗时 anim_speed 毫秒，ws2812fx_service() 大约每 10ms 调用一次。
    */
    static volatile uint8_t next_idx = 1;  // 下一个颜色索引
    static volatile float radian = 0.0f;   // 正弦角度（弧度值），范围 [0, 2PI)
    static volatile float radian_cnt = 0.0f;
    static volatile u32 color_0;
    static volatile u32 color_1;
    static volatile u8 dir = 0; // 亮度变化方向，0：渐亮，1：渐暗

    const float TWO_PI = 6.28318530717958647692f;

    volatile u32 color;
    volatile float steps;

    volatile float delta;
    volatile float t;
    volatile uint8_t blend;

    /*
        原本支持的动画速度：200 ~ 5000
        现在根据传递过来的速度对应的百分比值，映射到 200 ~ 5000
    */
    anim_speed = 5000 - (u32)fc_effect.report_speed * (5000 - 200) / 100;
    steps = (float)anim_speed / 10.0f; // 每 10ms 一步
    if (steps < 1.0f)
    {
        steps = 1.0f;
    }

    delta = TWO_PI / steps;

    if (0 == _seg_rt->counter_mode_step &&
        0 == _seg_rt->aux_param &&
        0 == _seg_rt->counter_mode_call)
    {
        // 刚进入该动画 
        // idx = 0;
        next_idx = (_seg->c_n > 1) ? 1 : 0;
        radian = 0.0f;
        radian_cnt = 0.0f;

        dir = 0;

        // color_0 = _seg->colors[idx % _seg->c_n];
        color_0 = _seg->colors[0];
        color_1 = _seg->colors[next_idx % _seg->c_n];
        _seg_rt->aux_param = 1; // 表示当前颜色索引已经初始化

        // printf("anim begin\n");
    }

    /* 取当前两色并按正弦插值计算混合比例（0..255） */
    t = sinf(radian);
    t = (t > 0) ? t : -t;
    blend = (uint8_t)((float)t * 255.0f); // t 从 0 -> 1 -> 0时，blend 从 0 -> 255 -> 0

    if (dir == 0)
    {
        // blend 从 0 -> 255
        color = WS2812FX_color_blend(color_0, color_1, blend);
    }
    else
    {
        // blend 从 255 -> 0，颜色 传参顺序要反过来
        color = WS2812FX_color_blend(color_1, color_0, blend);
    }

    Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

    // printf("color == %06x\n", color);
    // printf("blend == %u\n", (u16)blend);
    // printf("radian == %lu\n", (u32)(radian * 1000));

    /* 前进角度，完成一次 PI 后切换到下一对颜色 */
    radian += delta;
    radian_cnt += delta;

    if ((radian_cnt >= (TWO_PI / 4)) ||
        (dir == 0 && blend == 255) ||
        (dir == 1 && blend == 0))
    {
        /*
            1/4 pi 时，blend 最接近 255，应该切换到下一个颜色
            radian 在 0 ~ 2pi 变化期间，每经过 1/4 pi，就切换一次颜色
        */
        radian_cnt = 0;

        dir = !dir;

        next_idx = (next_idx + 1) % _seg->c_n;
        color_0 = color; // 从当前颜色开始往下一个颜色变化，防止颜色骤变
        color_1 = _seg->colors[next_idx];

        // printf("color change\n");
    }

    if (radian >= TWO_PI)
    {
        radian = 0;
        radian_cnt = 0;
        // printf("circle\n");
    }


    // return 100; // 测试时使用，观察颜色和数值变化
    return 1;
}

enum
{
    ANIM_INDEX_JUMP = 0, // 跳变
    ANIM_INDEX_SLIDE, // 滑翔
    ANIM_INDEX_BREATH, // 呼吸
    ANIM_INDEX_GRADUAL, // 渐变
    ANIM_INDEX_GRADUAL_BY_PAUSE, // 带停顿的渐变
};
typedef u8 anim_index_e;

u16 led_colorful_anim_auto(void)
{
    static volatile anim_index_e anim_index = 0; // 控制动画索引
    // static volatile u8 is_anim_jump_initialized = 0; // 
    static volatile u8 is_anim_slide_initialized = 0;
    static volatile u8 is_anim_breath_initialized = 0;
    static volatile u8 is_anim_gradual_initialized = 0;
    static volatile u8 is_anim_gradual_by_pause_initialized = 0;


    if (_seg_rt->aux_param == 0)
    {
        // 如果刚进入该函数，初始化参数
        _seg_rt->aux_param = 1;

        is_anim_slide_initialized = 0;
        is_anim_breath_initialized = 0;
        is_anim_gradual_initialized = 0;
        is_anim_gradual_by_pause_initialized = 0;


        anim_index = ANIM_INDEX_JUMP;
        printf("anim begin\n");
    }

    if (ANIM_INDEX_JUMP == anim_index)
    {
        Adafruit_NeoPixel_fill(
            _seg->colors[_seg_rt->counter_mode_step],
            _seg->start,
            _seg_len);
        _seg_rt->counter_mode_step++;
        _seg_rt->counter_mode_step %= _seg->c_n;

        if (_seg_rt->counter_mode_step == 0)
        {
            // 跳变完成，切换到下一种动画
            anim_index = ANIM_INDEX_SLIDE;
            printf("ANIM_INDEX_SLIDE \n");
            return 1; // 
        }

        return _seg->speed;
    }
    else if (ANIM_INDEX_SLIDE == anim_index)
    {
        // 七色滑翔使用的变量：
        u16 anim_speed = 0;
        /*
            使用正弦插值在两种颜色之间平滑过渡，形成滑翔效果。
            每次过渡耗时 anim_speed 毫秒，ws2812fx_service() 大约每 10ms 调用一次。
        */
        static volatile uint8_t next_idx = 1;  // 下一个颜色索引
        static volatile float radian = 0.0f;   // 正弦角度（弧度值），范围 [0, 2PI)
        static volatile float radian_cnt = 0.0f;
        static volatile u32 color_0;
        static volatile u32 color_1;
        static volatile u8 dir = 0; // 亮度变化方向，0：渐亮，1：渐暗 
        const float TWO_PI = 6.28318530717958647692f;
        volatile u32 color;
        volatile float steps;
        volatile float delta;
        volatile float t;
        volatile uint8_t blend;

        if (0 == is_anim_slide_initialized)
        {
            // 七色滑翔，参数初始化
            next_idx = (_seg->c_n > 1) ? 1 : 0;
            radian = 0.0f;
            radian_cnt = 0.0f;
            dir = 0;
            color_0 = _seg->colors[0];
            color_1 = _seg->colors[next_idx % _seg->c_n];

            is_anim_slide_initialized = 1;
        }

        /*
            原本支持的动画速度：200 ~ 5000
            现在根据传递过来的速度对应的百分比值，映射到 200 ~ 5000
        */
        anim_speed = 5000 - (u32)fc_effect.report_speed * (5000 - 200) / 100;
        steps = (float)anim_speed / 10.0f; // 每 10ms 一步
        if (steps < 1.0f)
        {
            steps = 1.0f;
        }

        delta = TWO_PI / steps;

        /* 取当前两色并按正弦插值计算混合比例（0..255） */
        t = sinf(radian);
        t = (t > 0) ? t : -t;
        blend = (uint8_t)((float)t * 255.0f); // t 从 0 -> 1 -> 0时，blend 从 0 -> 255 -> 0

        if (dir == 0)
        {
            // blend 从 0 -> 255
            color = WS2812FX_color_blend(color_0, color_1, blend);
        }
        else
        {
            // blend 从 255 -> 0，颜色 传参顺序要反过来
            color = WS2812FX_color_blend(color_1, color_0, blend);
        }

        Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

        /* 前进角度，完成一次 PI 后切换到下一对颜色 */
        radian += delta;
        radian_cnt += delta;

        if ((radian_cnt >= (TWO_PI / 4)) ||
            (dir == 0 && blend == 255) ||
            (dir == 1 && blend == 0))
        {
            /*
                1/4 pi 时，blend 最接近 255，应该切换到下一个颜色
                radian 在 0 ~ 2pi 变化期间，每经过 1/4 pi，就切换一次颜色
            */
            radian_cnt = 0;

            dir = !dir;

            next_idx = (next_idx + 1) % _seg->c_n;
            color_0 = color; // 从当前颜色开始往下一个颜色变化，防止颜色骤变
            color_1 = _seg->colors[next_idx];

            if (next_idx == 0)
            {
                // 滑翔完成，切换到下种动画
                anim_index = ANIM_INDEX_BREATH;
                printf("ANIM_INDEX_BREATH\n");
            }
        }

        if (radian >= TWO_PI)
        {
            radian = 0;
            radian_cnt = 0;
        }

        return 1;
    }
    else if (ANIM_INDEX_BREATH == anim_index)
    {
        u8 brightness_max = fc_effect.b;
        u16 anim_speed = 0;
        /*
            原本支持的动画速度：200 ~ 5000
            现在根据传递过来的速度对应的百分比值，映射到 200 ~ 5000
        */
        anim_speed = 5000 - (u32)fc_effect.report_speed * (5000 - 200) / 100;

        static u32 dest_color = BLACK; // 目标颜色  
        static volatile u32 temp_step = 0;  // 累计放大了1000倍的步长，超过1000后，才执行动画的下一步骤
        static volatile u16 brightness = 0; // 亮度值
        static volatile u8 cur_color_idx = 0;

        u32 step = 0;
        step = ((u32)brightness_max + 1) * 10 * 1000 / anim_speed; // 步长（放大了1000倍） 

        if (0 == is_anim_breath_initialized)
        {
            /*
                如果是第一次进入，设置默认颜色
                当前颜色为黑色，向目标颜色渐变（看起来像呼吸渐亮）
            */
            cur_color_idx = 0;
            dest_color = _seg->colors[cur_color_idx];
            brightness = 0;
            temp_step = 0;

            is_anim_breath_initialized = 1;
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

                    cur_color_idx += 1; // 切换颜色数组 _seg->colors[] 中的下一个颜色
                    if (cur_color_idx >= _seg->c_n)
                    {
                        cur_color_idx = 0;
                        // 切换到下一个动画
                        anim_index = ANIM_INDEX_GRADUAL;
                        return 1;
                    }

                    dest_color = _seg->colors[cur_color_idx];
                    SET_CYCLE;
                }
            }
        }

        u32 color = WS2812FX_color_blend(BLACK, dest_color, (u8)brightness);
        Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

        return 1; // ws2812fx_service() 10ms调用一次，这个值只需要小于10
    }
    else if (ANIM_INDEX_GRADUAL == anim_index)
    {
        static uint32_t c0, c1;
        static volatile u8 cur_color_idx = 0;

        if (0 == is_anim_gradual_initialized)
        {
            cur_color_idx = 0;
            c1 = _seg->colors[cur_color_idx];
            cur_color_idx++;
            c0 = _seg->colors[cur_color_idx];
            is_anim_gradual_initialized = 1;
        }

        int lum = _seg_rt->counter_mode_step;
        if (lum > 255) lum = 511 - lum; // lum = 0 -> 255 -> 0 
        // _seg->colors[1]:目标颜色
        uint32_t color = WS2812FX_color_blend(c1, c0, lum);
        Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

        if (_seg_rt->counter_mode_step == 256)
        {
            cur_color_idx++;
            cur_color_idx %= _seg->c_n;
            if (cur_color_idx == 0)
            {
                // 切换到下一种动画
                anim_index = ANIM_INDEX_GRADUAL_BY_PAUSE;
                return 1;
            }

            c1 = _seg->colors[cur_color_idx];
        }

        _seg_rt->counter_mode_step++;
        if (_seg_rt->counter_mode_step > 511)
        {
            _seg_rt->counter_mode_step = 0;
            cur_color_idx++;
            cur_color_idx %= _seg->c_n;
            c0 = _seg->colors[cur_color_idx];
            if (cur_color_idx == 0)
            {
                // 切换到下一种动画
                anim_index = ANIM_INDEX_GRADUAL_BY_PAUSE;
                return 1;
            }
            SET_CYCLE;
        }

        return (_seg->speed / 5);
    }
    else if (ANIM_INDEX_GRADUAL_BY_PAUSE == anim_index)
    {
        // static uint8_t index = 0;
        static volatile uint8_t cur_color_idx = 0;
        static uint32_t c0, c1;
        uint32_t color;
        u16 anim_speed;
        int lum = _seg_rt->counter_mode_step;
        /*
            原本返回的速度值范围：10 ~ 500 ，
            现在根据传递过来的速度对应的百分比值，映射到 10 ~ 500 ，
        */
        anim_speed = (500 - (u32)fc_effect.report_speed * (500 - 10) / 100);

        if (lum > 255)
        {
            // lum = 0 -> 255 -> 0
            lum = 511 - lum;
        }

        if (0 == is_anim_gradual_by_pause_initialized)
        {
            // 如果刚进入该函数，初始化参数 
            cur_color_idx = 0;
            c1 = _seg->colors[cur_color_idx];
            cur_color_idx++;
            c0 = _seg->colors[cur_color_idx];

            is_anim_gradual_by_pause_initialized = 1;
        }

        // 刚进入该函数时， _seg->colors[1]:目标颜色
        color = WS2812FX_color_blend(c1, c0, lum);
        Adafruit_NeoPixel_fill(color, _seg->start, _seg_len);

        if (_seg_rt->counter_mode_step == 256)
        {
            cur_color_idx++;
            if (cur_color_idx >= _seg->c_n)
            {
                anim_index = ANIM_INDEX_JUMP;
                return 1;
            }

            c1 = _seg->colors[cur_color_idx];
            _seg_rt->counter_mode_step++;

            // 变化完成一个颜色后，延时一段时间，再开始变换下一个颜色
            return (anim_speed * 100);
        }

        _seg_rt->counter_mode_step++;
        if (_seg_rt->counter_mode_step > 511)
        {
            _seg_rt->counter_mode_step = 0;
            cur_color_idx++;
            if (cur_color_idx >= _seg->c_n)
            {
                anim_index = ANIM_INDEX_JUMP;
                return 1;
            }

            c0 = _seg->colors[cur_color_idx];
        }

        return (anim_speed);
    }

    return 1;
}
