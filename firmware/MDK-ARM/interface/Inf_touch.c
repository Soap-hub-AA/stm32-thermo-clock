#include "Inf_touch.h"

uint8_t touch_value = 2;

/**
 * @brief 获取当前触摸开关的值
 * 
 * @return
 */

Touch_value Inf_get_touch_value(void)
{
    //1. 初始化读取 => 直接记录当前状态 不判断是否被触摸
    if (touch_value == 2)
    {
        touch_value = (uint8_t)HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin);
        //枚举类型
        return NO_TOUCH;

    }

    if (touch_value != (uint8_t)HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin))
    {
        //被触摸了
        touch_value = (uint8_t)HAL_GPIO_ReadPin(TOUCH_GPIO_Port, TOUCH_Pin);

        return TOUCH;
    }
    return NO_TOUCH;
}
