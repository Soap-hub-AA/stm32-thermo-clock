#ifndef __INF_NV020D__
#define __INF_NV020D__

#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Inf_DS1302z.h"
#include "com_debug.h"

#define NV020D_CLK_H HAL_GPIO_WritePin(NVD_CLK_GPIO_Port, NVD_CLK_Pin, GPIO_PIN_SET)
#define NV020D_CLK_L HAL_GPIO_WritePin(NVD_CLK_GPIO_Port, NVD_CLK_Pin, GPIO_PIN_RESET)

#define NV020D_DATA_H HAL_GPIO_WritePin(NVD_SDA_GPIO_Port, NVD_SDA_Pin, GPIO_PIN_SET)
#define NV020D_DATA_L HAL_GPIO_WritePin(NVD_SDA_GPIO_Port, NVD_SDA_Pin, GPIO_PIN_RESET)

#define NV020D_BUSY_READ HAL_GPIO_ReadPin(NVD_BUSY_GPIO_Port, NVD_BUSY_Pin)

//单线模式 IOA2 是数据控制线
#define NV020D_CTR_H HAL_GPIO_WritePin(NVD_CLK_GPIO_Port, NVD_CLK_Pin, GPIO_PIN_SET)
#define NV020D_CTR_L HAL_GPIO_WritePin(NVD_CLK_GPIO_Port, NVD_CLK_Pin, GPIO_PIN_RESET)

/**
 * @brief 初始化延时 有条件的加宽到500ms
 * 
 */
void Inf_nv020d_init(void);

/**
 * @brief 写入一个指令
 * 
 * @param cmd
 */
void Inf_nv020d_send_cmd(uint8_t cmd);

/**
 * @brief 设置音量 0-7
 * 
 * @param volume
 */
void Inf_nv020d_set_volume(uint8_t volume);

/**
 * @brief 发送一个空指令 =>停止播放
 * 
 */
void Inf_nv020d_stop(void);

#endif //  __INF_NV020D__
