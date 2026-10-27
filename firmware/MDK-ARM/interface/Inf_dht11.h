#ifndef __INF_DHT11___
#define __INF_DHT11___

#include "gpio.h"
#include "Inf_DS1302Z.h"
#include "FreeRTOS.h"
#include "task.h"


#define DHT11_DATA_H HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET)
#define DHT11_DATA_L HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_RESET)
//读取data引脚值
#define DHT11_DATA_Read HAL_GPIO_ReadPin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin)

/**
 * @brief DHT11模块上电之后需要延时1s才能使用
 * 
 */

void Inf_dht11_init(void);



/**
 * @brief 获取DHT11模块的温湿度数据   只保留整数部分即可  温度要确定正负
 * 
 * @param temperature  
 * @param humidity 
 */
void Inf_dht_get_data(int8_t *temperature, int8_t *humidity);


#endif  //__INF_DHT11___
