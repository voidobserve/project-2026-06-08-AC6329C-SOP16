/*
适用用2.4G遥控
基于中道版本2.4G遥控
1、app_config.h,把宏CONFIG_BT_GATT_CLIENT_NUM设置1
2、apps\spp_and_le\examples\trans_data\ble_trans.c  @bt_ble_init() 加入multi_client_init()
3、@le_gatt_client.c
   __resolve_adv_report()
   HCI_EIR_DATATYPE_MORE_16BIT_SERVICE_UUIDS 加入键值处理函数
4、在key_driver.c 注册rf24g_scan_para
5、board_ac632n_demo_cfg.h 使能TCFG_RF24GKEY_ENABLE
6、@app_tuya.c tuya_key_event_handler()加入上层应用的键值处理函数
7、底层无法判断长按，需要靠上层应用实现

以上该思路方法，在 CONFIG_APP_SPP_LE 这个demo上，实现起来很麻烦，不建议使用
 */


#include "system/includes.h"

#include "task.h"
#include "event.h"
#include "rf24g.h"
#include "led_strip_sys.h"
#include "board_ac632n_demo_cfg.h"
#include "led_strand_effect.h"
 // #include "tuya_ble_type.h"
// #include "one_wire.h"
#include "btstack/btstack_typedef.h"
#include "ble_multi_profile.h"
#include "att.h"
#include "asm/mcpwm.h"
// #if TCFG_RF24GKEY_ENABLE
// #include "ir_key_app.h"


#include "user_include.h"
#include "led_driver.h"
#include "rf24g_driver.h"

#include "led_strand_effect.h"
#include "user_report_app.h"

// #if 1
// #pragma pack (1)
// typedef struct
// {
//     u8 pair[3];
//     u8 flag;    //0:表示该数组没使用，0xAA：表示改数组已配对使用
// }rf24g_pair_t;
// #pragma pack ()
/***********************************************************移植须修改****************************************************************/

// #define PAIR_TIME_OUT 5*1000    //3秒
// static u16 pair_tc = 0;
// // 配对计时，10ms计数一次
// void rf24g_pair_tc(void)
// {
//     if (pair_tc <= PAIR_TIME_OUT)
//     {
//         pair_tc += 10;
//     }
// }

// #define PAIR_MAX    1

/***********************************************************移植须修改 END****************************************************************/

// rf24g_pair_t rf24g_pair[PAIR_MAX];        //需要写flash


/***********************************************************API*******************************************************************/




//-------------------------------------------------效果



// -----------------------------------------------声控


// -----------------------------------------------灵敏度




/***********************************************************APP*******************************************************************/

// extern rf24g_ins_t rf24g_ins;
// pair_handle是长按执行，长按时会被执行多次
// 所以执行一次后，要把pair_tc = PAIR_TIME_OUT，避免误触发2次
// static void pair_handle(void)
// {
//     extern void save_rf24g_pair_data(void);
//     u8 op = 0;//1:配对，2：解码
//     u8 i;

// }

u8 off_long_cnt = 0;
extern void parse_zd_data(unsigned char* LedCommand);
extern void set_IS_light_scene_state(void);
u8 all_mode[3] = { 0x04, 0x02, 0x07 };  //j模式集合
u8 sevrn_color_breath[3] = { 0x04, 0x02, 0x0b };  //七色呼吸
u8 stepmotpor_speed_cnt = 0;
u8 dynamic_speed = 0;
u8 meteor_flag;
u8 meteor_music_flag;
u8 meteor_speed[5] = { 1, 25, 50, 75, 100 }; //1, 25, 50, 75, 100
u8 meteor_cycle[5] = { 2, 8, 12, 16, 20 }; //2s 8s 12s 16s 20s
u8 cycle_cntt = 0;
u8 meteor_tail = 0;
u8 meteor_direction = 0;  //0：顺向  1：正向
u8 single_meteor = 0;
u8 fc_music_cnt = 0;
extern u8 Ble_Addr[6]; //蓝牙地址
extern hci_con_handle_t fd_handle;



void rf24_key_handle(struct sys_event* event)
{
    u8 event_type = 0;
    u8 key_value = 0;
    uint8_t Send_buffer[50];        //发送缓存.
    u8 temp[3] = { 0x04, 0x02, 0x08 };
    event_type = event->u.key.event;
    key_value = event->u.key.value;

    // printf("\n event->u.key.type = %d",event->u.key.type);
    // printf("\n event->event_type = %d",event_type);
    printf("key_value = %d\n", key_value);

    if (event->u.key.type == KEY_DRIVER_TYPE_RF24GKEY)  //按键类型
    {

        //开/关
        if (key_value == RF24_ON_OFF && event_type == KEY_EVENT_CLICK)
        {

            if (fc_effect.on_off_flag == DEVICE_ON)
                soft_rurn_off_lights();  //关灯.
            else
                soft_turn_on_the_light();  //开灯
        }

        if (key_value == RFKEY_ON_OFF)
        {
            printf("\n key_value = %d", key_value);

            if (fc_effect.on_off_flag == DEVICE_OFF) soft_turn_on_the_light();  //开灯
            else if (fc_effect.on_off_flag == DEVICE_ON) soft_rurn_off_lights();  //关灯.
        }

        if (get_on_off_state())
        {
            if (
                key_value != RFKEY_PAUSE &&
                key_value != RFKEY_ON_OFF &&
                key_value != RF24_ON_OFF &&
                key_value != RFKEY_LIGHT_PLUS &&
                key_value != RFKEY_LIGHT_SUB &&
                key_value != RFKEY_SPEED_PLUS &&

                key_value != RFKEY_SPEED_SUB &&
                key_value != RFKEY_2H &&
                key_value != RFKEY_1H
                )
            {
                // set_ir_auto(IR_PAUSE);
            }

            //速度/亮度 -
            if ((key_value == RF24_SPEED_BRIGHT_SUB || key_value == RFKEY_LIGHT_SUB) && event_type == KEY_EVENT_CLICK)
            {
                if (fc_effect.Now_state == IS_STATIC)
                {

                    extern void bright_sub(void);
                    bright_sub();
                    // memcpy(Send_buffer,Ble_Addr, 6);
                    // Send_buffer[6] = 0x04;
                    // Send_buffer[7] = 0x03;
                    // Send_buffer[8] = fc_effect.b*100/255;;
                    // ble_comm_att_send_data(fd_handle, ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer, 9, ATT_OP_AUTO_READ_CCC);
                    save_user_data_area3();
                }
                if (fc_effect.Now_state == IS_light_scene)
                {
                    if (MODE_MIXED_WHITE_BREATH == fc_effect.dream_scene.change_type)
                    {
                        // 如果正处于混白色呼吸 
                        fc_effect.dream_scene.mixed_white_breath_speed = (u16)4000;
                    }
                    else
                    {
                        fc_effect.dream_scene.speed = 350;
                    }

                    set_fc_effect();
                    save_user_data_area3();
                }
                if (fc_effect.Now_state == IS_light_music)
                {

                    extern void ls_sensitive_plus(void);
                    ls_sensitive_plus();
                    // Send_buffer[6] = 0x2F;
                    // Send_buffer[7] = 0x05;
                    // Send_buffer[8] = 100-fc_effect.sound.sensitive;
                    // // app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                    // ble_comm_att_send_data(fd_handle, ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer, 9, ATT_OP_AUTO_READ_CCC);
                    save_user_data_area3();

                }





            }


            //速度/亮度 +
            if ((key_value == RF24_SPEED_BRIGHT_PLUS || key_value == RFKEY_LIGHT_PLUS) && event_type == KEY_EVENT_CLICK)
            {
                if (fc_effect.Now_state == IS_STATIC)
                {

                    extern void bright_plus(void);
                    bright_plus();
                    // memcpy(Send_buffer,Ble_Addr, 6);
                    // Send_buffer[6] = 0x04;
                    // Send_buffer[7] = 0x03;
                    // Send_buffer[8] = fc_effect.b*100/255;;
                    // ble_comm_att_send_data(fd_handle, ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer, 9, ATT_OP_AUTO_READ_CCC);
                    save_user_data_area3();
                }
                if (fc_effect.Now_state == IS_light_scene)
                {
                    if (MODE_MIXED_WHITE_BREATH == fc_effect.dream_scene.change_type)
                    {
                        // 如果正处于混白色呼吸 
                        fc_effect.dream_scene.mixed_white_breath_speed = (u16)8000;
                    }
                    else
                    {
                        fc_effect.dream_scene.speed = 200;
                    }


                    set_fc_effect();
                    save_user_data_area3();
                }

                if (fc_effect.Now_state == IS_light_music)
                {

                    extern void ls_sensitive_sub(void);
                    ls_sensitive_sub();
                    // Send_buffer[6] = 0x2F;
                    // Send_buffer[7] = 0x05;
                    // Send_buffer[8] = 100-fc_effect.sound.sensitive;
                    // // app_send_user_data(ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer,9, ATT_OP_AUTO_READ_CCC);
                    // ble_comm_att_send_data(fd_handle, ATT_CHARACTERISTIC_fff1_01_VALUE_HANDLE, Send_buffer, 9, ATT_OP_AUTO_READ_CCC);
                    save_user_data_area3();

                }




            }
            //红色
            if (key_value == RF24_RED && event_type == KEY_EVENT_CLICK)
            {
                fc_static_effect(0);
                save_user_data_area3();

            }
            //绿色
            if (key_value == RF24_GREEN && event_type == KEY_EVENT_CLICK)
            {
                fc_static_effect(1);
                save_user_data_area3();

            }
            //蓝色
            if (key_value == RF24_BLUE && event_type == KEY_EVENT_CLICK)
            {
                fc_static_effect(2);
                save_user_data_area3();

            }
            //黄色
            if (key_value == RF24_YELLOW && event_type == KEY_EVENT_CLICK)
            {

                fc_static_effect(4);
                save_user_data_area3();

            }
            //天蓝色  85 250 255
            if (key_value == RF24_AZURE && event_type == KEY_EVENT_CLICK)
            {
                // set_static_mode(85, 250, 255);
                // 改成了 CYAN ， 蓝色和绿色分量最亮：
                set_static_mode(0, 255, 255);

                save_user_data_area3();
            }
            //玫红色  25 50 218  
            if (key_value == RF24_ROSE_RED && event_type == KEY_EVENT_CLICK)
            {
                // set_static_mode(255, 50, 218);
                // 改成 MAGENTA ， 红色和蓝色分量最亮
                set_static_mode(255, 0, 255);
                save_user_data_area3();
            }
            //纯白色   w b
            if (key_value == RF24_WHITE && event_type == KEY_EVENT_CLICK)
            {
                fc_static_effect(3);
                save_user_data_area3();

            }
            //模暖白光
            if (key_value == RF24_WARM_WHITE && event_type == KEY_EVENT_CLICK)
            {

                set_static_mode(255, 180, 20);
                save_user_data_area3();

            }
            //模式集合区
            if (key_value == RF24_ALL_MODE && event_type == KEY_EVENT_CLICK)
            {
                parse_zd_data(all_mode);
                all_mode[2] += 1;
                if (all_mode[2] > 0x1c)
                {
                    all_mode[2] = 0;
                }


            }
            //7色渐变
            if (key_value == RF24_SEVEN_COLOR_GRADUAL && event_type == KEY_EVENT_CLICK)
            {
                u8 temp[3] = { 0x04, 0x02, 0x0A };
                parse_zd_data(temp);

            }
            //七色呼吸
            if (key_value == RF24_SEVEN_COLOR_BREATHE && event_type == KEY_EVENT_CLICK)
            {
                // printf("breath\n");
                u8 temp_buff[3] = { 0x04, 0x02, 254 }; // 对应混白色呼吸


                parse_zd_data(temp_buff);

#if 0
                parse_zd_data(sevrn_color_breath);
                sevrn_color_breath[2] += 1;
                if (sevrn_color_breath[2] > 0x11)
                {
                    sevrn_color_breath[2] = 0x0B;
                }
#endif

            }
            //7色跳变
            if (key_value == RF24_SEVEN_COLOR_JUMP && event_type == KEY_EVENT_CLICK)
            {
                u8 temp[3] = { 0x04, 0x02, 0x08 };
                parse_zd_data(temp);

            }
            //电机转速调节 5挡   8s 13s 18s 21s 26s 35s
            if (key_value == RF24_STEMPMOTOR_SPEED && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO Motor_Switch 函数已注释，待添加新的接口，如果遥控器没有对应的模式，可以直接注释
                // extern void Motor_Switch(void);
                // Motor_Switch();
            }
            //声控1    4音乐律动减
            if (key_value == RF24_SOUND_ONE && event_type == KEY_EVENT_CLICK)
            {

                if (fc_music_cnt > 0)
                    fc_music_cnt--;
                set_music_mode(fc_music_cnt);

            }

            //声控2   4音乐律动加
            if (key_value == RF24_SOUND_TWO && event_type == KEY_EVENT_CLICK)
            {

                if (fc_music_cnt < 3)
                    fc_music_cnt++;

                set_music_mode(fc_music_cnt);

            }

            if (key_value == RF24_ONE_TOW_METEOR && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
                // one_wire_set_period(period[0]);
                save_user_data_area3();

            }
            if (key_value == RF24_METEOR_SOUND_ONE_TWO && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
   // one_wire_set_period(period[1]);
                save_user_data_area3();
            }
            if (key_value == RF24_DIRECTION && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
                   // one_wire_set_period(period[2]);
                save_user_data_area3();
            }
            if (key_value == RF24_METEOR_SPEED && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
                   // one_wire_set_period(period[3]);
                save_user_data_area3();
            }
            if (key_value == RF24_METEOR_FREQUENCY && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
   // one_wire_set_period(period[4]);
                save_user_data_area3();
            }
            if (key_value == RF24_METEOR_TAIL && event_type == KEY_EVENT_CLICK)
            {
                // USER_TO_DO one_wire_set_period 已屏蔽，待添加新的接口
   // one_wire_set_period(period[5]);
                save_user_data_area3();
            }


        }

    }


}




extern const rf24_key_handle_func_t rf24g_key_type_28keys_handle_func_buff[RF24G_TYPE_28KEY_EVENT_MAX]; // 变量声明

void rf24g_key_handle(void)
{
    u8 rf24g_key_event = 0;
    rf24_key_handle_func_t rf24g_key_handle_func_ptr = NULL;

    rf24g_key_event = rf24g_convert_to_key_event(rf24g_key.driver_key_val, rf24g_key.driver_key_event);
    rf24g_key.driver_key_val = NO_KEY; // 置为无效键值（由于扫描函数只更新，不会清除，在这里要清除）

    if (rf24g_key_event == RF24G_TYPE_28KEY_EVENT_NONE)
    {
        // 如果是无效的按键事件，直接返回
        return;
    }

    // printf("rf24g_key_event: %u\n", (u16)rf24g_key_event);

    // 调用按键事件对应的处理函数
    rf24g_key_handle_func_ptr = rf24g_key_type_28keys_handle_func_buff[rf24g_key_event];
    if (NULL == rf24g_key_handle_func_ptr)
    {
        // 如果按键事件没有对应的处理函数，直接退出
        return;
    }

    // 直接调用对应的处理函数，这样需要每个处理函数内都要判断一下设备是否开机
    rf24g_key_handle_func_ptr();

    // 将变化的数据保存到flash
    os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
}

void rf24g_key_r1c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r1c1 press\n");
#endif

    // 静态模式下，调节亮度 
    // 声控模式下，调节灵敏度

    if (fc_effect.on_off_flag == DEVICE_OFF)
    {
        return;
    }

    if (fc_effect.Now_state == IS_STATIC)
    {
        bright_plus();
        user_report_brightness(fc_effect.b);
#if USER_DEBUG_ENABLE
        printf("fc_effect.b == %u\n", (u16)fc_effect.b);
#endif
    }
    else if (fc_effect.Now_state == IS_light_music)
    {
        ls_sensitive_plus();
        user_report_sound_control_sensitive(fc_effect.sound.sensitive);
#if USER_DEBUG_ENABLE
        printf("fc_effect.sound.sensitive == %u\n", (u16)fc_effect.sound.sensitive);
#endif
    }
}

void rf24g_key_r1c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r1c2 press\n");
#endif

    // 静态模式下，调节亮度 
    // 声控模式下，调节灵敏度

    if (fc_effect.on_off_flag == DEVICE_OFF)
    {
        return;
    }

    if (fc_effect.Now_state == IS_STATIC)
    {
        bright_sub();
        user_report_brightness(fc_effect.b);
#if USER_DEBUG_ENABLE
        printf("fc_effect.b == %u\n", (u16)fc_effect.b);
#endif
    }
    else if (fc_effect.Now_state == IS_light_music)
    {
        ls_sensitive_sub();
        user_report_sound_control_sensitive(fc_effect.sound.sensitive);
#if USER_DEBUG_ENABLE
        printf("fc_effect.sound.sensitive == %u\n", (u16)fc_effect.sound.sensitive);
#endif
    }
}

void rf24g_key_r1c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r1c3 press\n");
#endif

    // 关机 
    soft_rurn_off_lights();
}

void rf24g_key_r1c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r1c4 press\n");
#endif

    // 开机 
    soft_turn_on_the_light();
}

void rf24g_key_r2c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r2c1 press\n");
#endif

    // 红色
    led_colorful_light_set_static_color(RED);
}

void rf24g_key_r2c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r2c2 press\n");
#endif

    // 绿色
    led_colorful_light_set_static_color(GREEN);
}

void rf24g_key_r2c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r2c3 press\n");
#endif

    // 蓝色
    led_colorful_light_set_static_color(BLUE);
}

void rf24g_key_r2c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r2c4 press\n");
#endif

    if (led_driver.is_mixed_white_light)
    {
        // 如果是混白色灯，没有白色分量，所以将R、G、B都设置为最大，构成混白色
        led_colorful_light_set_static_color(WHITE);
    }
    else
    {
        // 纯白色
        led_colorful_light_set_static_color(PURE_WHITE);
    }
}

void rf24g_key_r3c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r3c1 press\n");
#endif

    // 黄色
    led_colorful_light_set_static_color(YELLOW);
}

void rf24g_key_r3c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r3c2 press\n");
#endif

    // 青色
    led_colorful_light_set_static_color(CYAN);
}

void rf24g_key_r3c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r3c3 press\n");
#endif

    // 紫色
    led_colorful_light_set_static_color(MAGENTA);
}

void rf24g_key_r3c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r3c4 press\n");
#endif

    if (led_driver.is_mixed_white_light)
    {
        // 如果是混白色灯，将R、G、B都设置为最大，构成混白色
        led_colorful_light_set_static_color(WHITE);
    }
    else
    {
        // 纯白色
        led_colorful_light_set_static_color(PURE_WHITE);
    }
}

void rf24g_key_r4c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r4c1 press\n");
#endif

    /*
        AUTO 模式，在下面5个模式不断循环切换
        七色跳变 -> 七色滑翔 -> 七色呼吸 -> 连续渐变 -> 渐变带停顿 -> ...
    */
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_COLORFUL_AUTO;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r4c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r4c2 press\n");
#endif

    /*
        七色跳变
    */
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_JUMP;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r4c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r4c3 press\n");
#endif 

    // 连续渐变
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_MUTIL_C_GRADUAL;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r4c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r4c4 press\n");
#endif

    // 动画 速度加
    if (fc_effect.Now_state == IS_light_scene)
    {
        ls_speed_plus();
#if USER_DEBUG_ENABLE
        printf("fc_effect.dream_scene.speed == %u\n",
            fc_effect.dream_scene.speed);
        printf("fc_effect.report_speed == %u\n",
            (u16)fc_effect.report_speed);
#endif
        set_fc_effect();
        user_report_speed(fc_effect.report_speed);
    }
}

void rf24g_key_r5c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r5c1 press\n");
#endif

    // 七色呼吸
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_COLORFUL_BREATH;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r5c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r5c2 press\n");
#endif

    // 七色滑翔
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_COLORFUL_SLIDE;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r5c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r5c3 press\n");
#endif

    // 七色 渐变带停顿 
    ls_set_color(0, BLUE);
    ls_set_color(1, GREEN);
    ls_set_color(2, RED);
    ls_set_color(3, WHITE);
    ls_set_color(4, YELLOW);
    ls_set_color(5, CYAN);
    ls_set_color(6, PURPLE);
    fc_effect.dream_scene.change_type = MODE_COLORFUL_GRADUAL_BY_PAUSE;
    fc_effect.dream_scene.c_n = 7;
    fc_effect.Now_state = IS_light_scene;
    set_fc_effect();
}

void rf24g_key_r5c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r5c4 press\n");
#endif

    // 动画速度减
    if (fc_effect.Now_state == IS_light_scene)
    {
        ls_speed_sub();
#if USER_DEBUG_ENABLE
        printf("fc_effect.dream_scene.speed == %u\n",
            fc_effect.dream_scene.speed);
        printf("fc_effect.report_speed == %u\n",
            (u16)fc_effect.report_speed);
#endif
        set_fc_effect();
        user_report_speed(fc_effect.report_speed);
    }
}

void rf24g_key_r6c1_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r6c1 press\n");
#endif

    // 声控模式，没有声音时灭灯
    fc_effect.music.m = 3;
    fc_effect.Now_state = IS_light_music;
    set_fc_effect();
    user_report_sound_control_mode(fc_effect.music.m);
}

void rf24g_key_r6c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r6c2 press\n");
#endif

    // 声控模式，没有声音的时候渐变
    fc_effect.music.m = 0;
    fc_effect.Now_state = IS_light_music;
    set_fc_effect();
    user_report_sound_control_mode(fc_effect.music.m);
}

void rf24g_key_r6c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r6c3 press\n");
#endif

    // 声控模式，没有声音的时候跳变

    // 实际是有声音的时候跳变
    fc_effect.music.m = 2;
    fc_effect.Now_state = IS_light_music;
    set_fc_effect();
    user_report_sound_control_mode(fc_effect.music.m);
}

void rf24g_key_r6c4_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r6c4 press\n");
#endif

    // 声控模式，没有声音的时候呼吸 
    fc_effect.music.m = 1;
    fc_effect.Now_state = IS_light_music;
    set_fc_effect();
    user_report_sound_control_mode(fc_effect.music.m);
}
void rf24g_key_r7c1_click_handle(void)
{
    const u8 step = 4;

#if USER_DEBUG_ENABLE
    printf("r7c1 click\n");
#endif

    fc_effect.motor_sec_per_round = 35;
    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}

void rf24g_key_r7c2_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r7c2 press\n");
#endif

    fc_effect.motor_sec_per_round = 21;
    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}

void rf24g_key_r7c3_press_handle(void)
{
#if USER_DEBUG_ENABLE
    printf("r7c3 press\n");
#endif

    fc_effect.motor_sec_per_round = 13;
    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}

void rf24g_key_r7c4_click_handle(void)
{
    const u8 step = 4;

#if USER_DEBUG_ENABLE
    printf("r7c4 click\n");
#endif

    fc_effect.motor_sec_per_round = 4;
    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}

void rf24g_key_r7c1_hold_handle(void)
{
    const u8 step = 1;

#if USER_DEBUG_ENABLE
    printf("r7c1 hold\n");
#endif

    // 电机速度 减
    if (fc_effect.motor_sec_per_round < 35 - step)
    {
        fc_effect.motor_sec_per_round += step;
    }
    else
    {
        fc_effect.motor_sec_per_round = 35;
    }

    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}

void rf24g_key_r7c4_hold_handle(void)
{
    const u8 step = 1;

#if USER_DEBUG_ENABLE
    printf("r7c4 hold\n");
#endif

    // 电机速度 加 
    if (fc_effect.motor_sec_per_round > 4 + step)
    {
        fc_effect.motor_sec_per_round -= step;
    }
    else
    {
        fc_effect.motor_sec_per_round = 4;
    }

    motor_set_speed_sec_per_round(fc_effect.motor_sec_per_round);
#if USER_DEBUG_ENABLE
    printf("fc_effect.motor_sec_per_round == %u\n",
        (u16)fc_effect.motor_sec_per_round);
#endif
}


const rf24_key_handle_func_t rf24g_key_type_28keys_handle_func_buff[RF24G_TYPE_28KEY_EVENT_MAX] = {
    [RF24G_TYPE_28KEY_EVENT_R1C1_PRESS] = rf24g_key_r1c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R1C2_PRESS] = rf24g_key_r1c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R1C3_PRESS] = rf24g_key_r1c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R1C4_PRESS] = rf24g_key_r1c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R2C1_PRESS] = rf24g_key_r2c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R2C2_PRESS] = rf24g_key_r2c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R2C3_PRESS] = rf24g_key_r2c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R2C4_PRESS] = rf24g_key_r2c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R3C1_PRESS] = rf24g_key_r3c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R3C2_PRESS] = rf24g_key_r3c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R3C3_PRESS] = rf24g_key_r3c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R3C4_PRESS] = rf24g_key_r3c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R4C1_PRESS] = rf24g_key_r4c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R4C2_PRESS] = rf24g_key_r4c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R4C3_PRESS] = rf24g_key_r4c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R4C4_PRESS] = rf24g_key_r4c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R5C1_PRESS] = rf24g_key_r5c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R5C2_PRESS] = rf24g_key_r5c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R5C3_PRESS] = rf24g_key_r5c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R5C4_PRESS] = rf24g_key_r5c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R6C1_PRESS] = rf24g_key_r6c1_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R6C2_PRESS] = rf24g_key_r6c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R6C3_PRESS] = rf24g_key_r6c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R6C4_PRESS] = rf24g_key_r6c4_press_handle,

    [RF24G_TYPE_28KEY_EVENT_R7C1_CLICK] = rf24g_key_r7c1_click_handle,
    [RF24G_TYPE_28KEY_EVENT_R7C2_PRESS] = rf24g_key_r7c2_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R7C3_PRESS] = rf24g_key_r7c3_press_handle,
    [RF24G_TYPE_28KEY_EVENT_R7C4_CLICK] = rf24g_key_r7c4_click_handle,

    [RF24G_TYPE_28KEY_EVENT_R7C1_HOLD] = rf24g_key_r7c1_hold_handle,
    [RF24G_TYPE_28KEY_EVENT_R7C4_HOLD] = rf24g_key_r7c4_hold_handle,
};
