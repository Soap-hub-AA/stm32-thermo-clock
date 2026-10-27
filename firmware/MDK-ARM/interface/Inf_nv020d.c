#include "Inf_nv020d.h"


/**
 * @brief 初始化延时 有条件的加宽到500ms
 * 
 */
void Inf_nv020d_init(void)
{
    vTaskDelay(500);
}

/**
 * @brief 写入一个字节
 * 
 * @param cmd
 */
void Inf_nv020d_send_byte(uint8_t data)
{
    //1.初始化过程
    NV020D_CLK_H;

    //提前设置好D1的值
    if (data & (1))
    {
        NV020D_DATA_H;
    }
    else
    {
        NV020D_DATA_L;
    }
    //2.发送起始命令
    NV020D_CLK_L;
    vTaskDelay(4);
    //循环发送指令
    for (uint8_t i = 0; i < 8; i++)
    {
        if (data & (1 << i))
        {
            NV020D_DATA_H;
        }
        else
        {
            NV020D_DATA_L;
        }

        NV020D_CLK_L;
        Inf_Delay_us(400);
        NV020D_CLK_H;
        Inf_Delay_us(400);
    }

    NV020D_DATA_H;

}

/**
 * @brief 写入一个指令
 * 
 * @param cmd
 */
// void Inf_nv020d_send_cmd(uint8_t cmd)
// {
//     //等待忙状态
//     while (NV020D_BUSY_READ == GPIO_PIN_RESET)
//     {
//         //间隔性释放CPU
//         vTaskDelay(10);
//     }
//     //两个指令之间最小间隔是100ms
//     vTaskDelay(100);

//     Inf_nv020d_send_byte(0xf1);
//     //0-0x0f 表示播放对应下标位置的语言
//     Inf_nv020d_send_byte(cmd);
//     Inf_nv020d_send_byte(0xf3);
//     uint8_t sum = (uint8_t)(0xf1 + cmd + 0xf3);
//     Inf_nv020d_send_byte(sum);
// }

/**
 * @brief 写入一个指令
 * 
 * @param cmd
 */
void Inf_nv020d_send_cmd(uint8_t cmd)
{
    //使用单线控制
    //初始化
    NV020D_CTR_H;

    //1.拉低控制线4-6ms
    NV020D_CTR_L;
    vTaskDelay(4);

    //2.发送数据 => 此处使用vtaskdelay(1)(3) => 虽然发送数据的时间长了 但是不阻塞CPU了
    //改完之后如果想要适配逻辑 => 调用这个函数方法的优先级一定要是最高的
    for (uint8_t i = 0; i < 8; i++)
    {
        if (cmd & (1 << i))
        {
            //发送1
            NV020D_CTR_H;//3:1
            //Inf_Delay_us(1200);
            vTaskDelay(3);
            NV020D_CTR_L;//1:1
            //Inf_Delay_us(400);
            vTaskDelay(1);
        }
        else
        {
            //发送0
            NV020D_CTR_H;// 1:3
            // Inf_Delay_us(400);
            vTaskDelay(1);
            NV020D_CTR_L;
            // Inf_Delay_us(1200);
            vTaskDelay(3);
        }
    }
    //拉高控制线
    NV020D_CTR_H;
    vTaskDelay(100);
}


/**
 * @brief 设置音量 0-7
 * 
 * @param volume
 */
void Inf_nv020d_set_volume(uint8_t volume)
{
    if (volume > 7)
    {
        debug_printf("volume is out of range");
        volume = 7;
    }

    Inf_nv020d_send_cmd(0xE0 + volume);
}


/**
 * @brief 发送一个空指令 =>停止播放
 * 
 */
void Inf_nv020d_stop(void)
{
    //可以添加一个判断 当前是否有效  busy线拉低的情况

    //也可以不判断 直接发送停止
    Inf_nv020d_send_cmd(0xfe);
}