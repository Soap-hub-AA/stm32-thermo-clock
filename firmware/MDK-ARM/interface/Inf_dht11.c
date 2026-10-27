#include "Inf_dht11.h"
#include "com_debug.h"

//用于存储每次读取到的值
uint8_t data[5] = {0};

/**
 * @brief DHT11模块上电之后需要延时1s才能使用
 * 
 */

void Inf_dht11_init(void)
{
    //健壮性冗余  确保DHT11高电平
    DHT11_DATA_H;
    //上电之后需要延时2s才能使用
    vTaskDelay(2000);  //使用FreeRTOS的延时必须在启动FreeRTOS之后
}



/**
 * @brief 获取DHT11模块的温湿度数据   只保留整数部分即可  温度要确定正负
 * 
 * @param temperature  
 * @param humidity 
 */
void Inf_dht_get_data(int8_t *temperature, int8_t *humidity)
{

    int8_t temp = 0;
    int8_t hum = 0;

    //1.发送起始信号 => 拉低引脚18-30ms => 标准值20ms
    DHT11_DATA_L;
    vTaskDelay(20);
    DHT11_DATA_H;

    //以下读取温湿度的代码 不能被打断  必须是连续地使用
    //添加临界区 => 临界区中的代码一定是连续执行的 代码原子性
    taskENTER_CRITICAL();


    //2.接受回顾的响应信号 => 回顾信号的高低电平时间有没有具体意义？ => 没有 唯一使用意义就是可以释放CPU到点了再回来看这个引脚
    //（ 时间不足1ms 释放CPU也无意义）
    //如果DHT11模块坏了 不能正确返回响应信号 => 写一个超时的逻辑 超时的时间大概是2s
    uint32_t count_max = 0xffffff;
    while(DHT11_DATA_Read == GPIO_PIN_SET && count_max--)
    {
        //等待响应信号拉低
    }
    while(DHT11_DATA_Read == GPIO_PIN_RESET && count_max--)
    {
        //发送低电平的响应信号  会持续83us
        //等待响应信号拉高
    }
    while(DHT11_DATA_Read == GPIO_PIN_SET && count_max--)
    {
        //发送高电平的响应信号  会持续87us
        //等待响应信号拉低
    }
    //3.判断是超时了 还是接受到正确的响应信号了
    if (count_max == 0)
    {
        debug_printf("DHT11 响应超时");
        return;
    }

    //4.接受数据 数据格式 5个字节 高位在前
    // uint8_t hum_int = 0;
    // for (int i = 0; i < 8; i++)
    // {
    //     //第一个字节
    //     //4.1首先是54us的低电平信号
    //     while (DHT11_DATA_Read == GPIO_PIN_RESET)
    //     {
    //         //等待拉高电平
    //     }
    //     // 延时27-68us  使用40us
    //     Inf_Delay_us(40);
    //     if (DHT11_DATA_Read == GPIO_PIN_SET)
    //     {
    //         //高电平  说明是1
    //         hum_int |= (1 << (7 - i));
    //         while (DHT11_DATA_Read == GPIO_PIN_SET)
    //         {
    //             //等待拉低进入下一轮读数
    //         }
    //     }
    //     else
    //     {
    //         //低电平  说明是0
    //         //不用写 => 因为hum_int已经是0了
    //     }
    // }

    //使用双重循环 => 写一遍代码 表示循环5次
    for (uint8_t i = 0; i < 5; i++)
    {
        data[i] = 0;
        for (uint8_t j = 0; j < 8; j++)
        {
            //等待拉高读取数据
            while (DHT11_DATA_Read == GPIO_PIN_RESET)
            {
                //等待拉高电平
            }
            Inf_Delay_us(40);
            if (DHT11_DATA_Read == GPIO_PIN_SET)
            {
                //读取数据为1 => 写入到数据当中
                data[i] |= (1 << (7 - j));
                //等待下降沿 => 进入下一轮读数
                while (DHT11_DATA_Read == GPIO_PIN_SET)
                {
                    //等待拉低进入下一轮读数
                }
            }
            else
            {
                //读取到的是0 data[0] = 0；已经重置为0
                //本身已经是下降沿 => 直接等待下一轮的上升沿即可 不需要操作
            }
        }
    }
    //5. 校验数据
    uint8_t sum = data[0] + data[1] + data[2] + data[3];
    if ((uint8_t)sum == data[4])
    {
        // 通过校验 => 可以赋值
        hum = data[0];

        // 赋值温度的整数部分
        temp = data[2];
        //判断正负
        if (data[3] & 0x80)
        {
            //负数
            temp = -temp;
        }

        //赋值返回
        *temperature = temp;
        *humidity = hum;

    }
    else
    {
        // 没有通过校验
        debug_printf("DHT11 校验失败");
    }
    taskEXIT_CRITICAL();
}
