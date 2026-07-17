
#include "system/includes.h"
#include "syscfg_id.h"
#include "save_flash.h"
#include "led_driver.h"
#include "Adafruit_NeoPixel.H"

#include "user_include.h"

#define CFG_USER_SAVE_DATA_PAGE_ID 3
#define USER_SAVE_DATA_VALID_CODE ((u8)0xC5) // 表示用户保存的数据有效的标志

/*
    保存用户数据，调用 os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    给线程发送消息，使能保存数据的功能，延时到来后，由用户主线程执行保存数据的操作
*/

volatile user_save_data_t save_data = { 0 };


void user_data_init(void)
{
    u8 sequence;

    int ret = 0;
    ret = syscfg_read(
        CFG_USER_SAVE_DATA_PAGE_ID,
        (void*)(&save_data),
        sizeof(user_save_data_t)
    );

#if USER_DEBUG_ENABLE
    printf("save_data.fc_save.motor_mode = %u\n", (u16)save_data.fc_save.motor_mode);
    printf("save_data.fc_save.motor_sec_per_round = %u\n", (u16)save_data.fc_save.motor_sec_per_round);
#endif

    if (save_data.data_valid_code != USER_SAVE_DATA_VALID_CODE)
    {
        // 数据无效，可能是第一次上电，或者是之前的数据损坏

        save_data.data_valid_code = USER_SAVE_DATA_VALID_CODE;
        fc_data_init();
        // 初始化完成后，将数据写回
        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);

#if USER_DEBUG_ENABLE
        printf("data init\n");
#endif
    }
    else
    {
        // 如果数据有效 
        memcpy(
            (void*)(&fc_effect),
            (void*)(&save_data.fc_save),
            sizeof(fc_effect_t));

#if USER_DEBUG_ENABLE
        printf("data load\n");
#endif
    }

    /*
        程序执行到这里时，要确保 led_driver.is_mixed_white_light 已经初始化
        要先调用 led_driver_init()
    */
    if (led_driver.is_mixed_white_light)
    {
        sequence = NEO_RGB; // 
    }
    else
    {
        sequence = NEO_RGBW;
    }

#if USER_DEBUG_ENABLE
    if (NEO_RGB == sequence)
    {
        printf("cur sequence == NEO_RGB\n");
    }
    else if (NEO_RGBW == sequence)
    {
        printf("cur sequence == NEO_RGBW\n");
    }
#endif

    if (sequence != fc_effect.sequence)
    {
        // 如果当前的RGB顺序和保存的数据不一致，则要修改为当前的 

        fc_effect.sequence = sequence;
        if (NEO_RGB == sequence)
        {
            fc_effect.rgb.w = 0; // 如果是混白色灯，W通道必须为0
            fc_effect.rgb.r = 0xFF;
            fc_effect.rgb.g = 0xFF;
            fc_effect.rgb.b = 0xFF;
        }
        else if (NEO_RGBW == sequence)
        {
            // 如果是纯白色灯（RGBW），初始只点亮纯白色
            fc_effect.rgb.w = 0xFF;
            fc_effect.rgb.r = 0x00;
            fc_effect.rgb.g = 0x00;
            fc_effect.rgb.b = 0x00;
        }
        else
        {
#if USER_DEBUG_ENABLE
            printf("error sequence\n");
#endif
        }

        os_taskq_post("msg_task", 1, MSG_USER_SAVE_INFO);
    }

    // USER_TO_DO 检查发现上电后，该变量不是电机正转，强制赋值为电机正转
    // fc_effect.motor_mode = MOTOR_MODE_FORWARD_ROTATION;
}

void user_data_save(void)
{
    int ret = 0;

    save_data.data_valid_code = USER_SAVE_DATA_VALID_CODE; // 表示数据有效

    memcpy(
        (void*)(&save_data.fc_save),
        (void*)(&fc_effect),
        sizeof(fc_effect_t));

    os_time_dly(1); // 先让出cpu，处理其他任务，防止看门狗复位
    // local_irq_disable(); // 禁用中断
    ret = syscfg_write(CFG_USER_SAVE_DATA_PAGE_ID, (u8*)(&save_data), sizeof(user_save_data_t));
    // local_irq_enable(); // 使能中断

#if USER_DEBUG_ENABLE
    printf("save info done \n");
    printf("fc_effect.motor_mode = %u\n", (u16)fc_effect.motor_mode);
    printf("fc_effect.motor_sec_per_round = %u\n", (u16)fc_effect.motor_sec_per_round);
    printf("save_data.fc_save.motor_mode = %u\n", (u16)save_data.fc_save.motor_mode);
    printf("save_data.fc_save.motor_sec_per_round = %u\n", (u16)save_data.fc_save.motor_sec_per_round);
#endif
}

void user_data_save_enable(void)
{
    save_data.is_save_enable = 1;
}

/**
 * @brief 如果使能了保存，累计时间，时间到来后，
 *          由 user_data_save_handle() 执行保存操作
 *      该函数 10ms 调用一次，不需要特别准确
 *
 */
void user_data_save_delay_add(void)
{
    if (save_data.is_save_enable)
    {
        if (save_data.delay_time_cnt < ((u16)-1) - 10)
        {
            save_data.delay_time_cnt += 10;
        }
    }
    else
    {
        save_data.delay_time_cnt = 0;
    }
}

void user_data_save_handle(void)
{
    // 判断是否要执行数据保存操作（判断保存时间间隔是否溢出）
    if (save_data.delay_time_cnt >= DELAY_SAVE_FLASH_TIMES)
    {
        save_data.delay_time_cnt = 0;
        save_data.is_save_enable = 0;
        user_data_save();
    }


}


// void read_flash_device_status_init(void)
// {
//     int ret;
//     save_flash_t save_flash3;
//     memset((u8*)&save_flash3, 0, sizeof(save_flash_t)); 

//     ret = syscfg_read(
//         CFG_USER_SAVE_DATA_PAGE_ID,
//         (u8*)(&save_flash3),
//         sizeof(save_flash_t));


//     if (save_flash3.header != USER_SAVE_DATA_VALID_CODE)  // 第一次上电
//     {
//         printf("first read flash");
//         fc_data_init();
//     }
//     else
//     {
//         memcpy((u8*)(&fc_effect), (u8*)(&save_flash3.fc_save), sizeof(fc_effect_t));
//     }
// }


// 把用户数据写到区域3
void save_user_data_area3(void)
{
    // save_flash_t save_data;
    save_data.data_valid_code = USER_SAVE_DATA_VALID_CODE;
    // 不保存开关机状态，默认开机
    // fc_effect.on_off_flag = DEVICE_ON;
    memcpy(
        (u8*)(&save_data.fc_save),
        (u8*)(&fc_effect),
        sizeof(fc_effect_t));
    syscfg_write(
        CFG_USER_SAVE_DATA_PAGE_ID,
        (u8*)(&save_data),
        sizeof(user_save_data_t));

}
















