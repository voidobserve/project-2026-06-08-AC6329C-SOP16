#ifndef __DET_50HZ_H__
#define __DET_50HZ_H__

#include "typedef.h"
#include "gpio.h"

#define DET_50HZ_PIN IO_PORT_DM

void det_50hz_init(void);

u32 det_50hz_get_tick(void);

#endif