#ifndef __INF_MIC__
#define __INF_MIC__


#include "gpio.h"

typedef enum
{
    MIC_ON,
    MIC_OFF,
} Mic_value;


/**
 * @brief 根据MIC_IN的值判断是否有人说话
 * 
 * 
 * @return Mic_value 1 表示有人说话 0 表示没有人说话
 */
Mic_value Inf_get_mic_value(void);


#endif  //  __INF_MIC__
