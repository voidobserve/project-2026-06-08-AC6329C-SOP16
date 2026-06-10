
#ifndef   __SAVE_FLASH_H__

#define   __SAVE_FLASH_H__
#include "led_strand_effect.h"
#include "led_strip_drive.h"

// #include "motor.h"
// #include "led_driver.h"

// 需要保存数据时，延时保存的时间：（单位：ms）
#define DELAY_SAVE_FLASH_TIMES ((u16)3000)

typedef enum
{
    FRIST_BYTE,         //第一次上电标志
    LED_LEDGTH_MS,      //灯带长度高8位
    LED_LEDGTH_LS,      //灯带长度低8位
}FLASH_BYTE_FLAG;

#pragma pack (1)
typedef struct
{
    unsigned char header;           //头部
    fc_effect_t fc_save;
}save_flash_t;

typedef struct
{
    u8 data_valid_code; // 存放数据是否有效的标志

    u8 is_save_enable; // 是否要保存数据
    u16 delay_time_cnt; // 每次保存数据的延时时间计数
    fc_effect_t fc_save;

} user_save_data_t;




#pragma pack ()

void save_user_data_area3(void);

void user_data_save_enable(u8 enable);
void user_data_save_delay_add(void);
void user_data_save_handle(void);

#endif