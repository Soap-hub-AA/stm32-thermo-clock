#include "App_freeRTOS.h"

//采集任务  ms级延时 => 不会影响整个程序运行 => 通常为低优先级
void task_collect(void *pvParameters);
#define COLLECT_TASK_STACK_SIZE 128
#define COLLECT_TASK_PRIORITY 1
TaskHandle_t collect_task_handle;

//全局变量保存温湿度的值 => 只有采集任务才能修改 => 其他任务只能读取
int8_t temperature = 0;
int8_t humidity = 0;

//全局变量保存时间信息
Clock_Date_time_type clock_date_time = {0};

//展示任务
void task_show(void *pvParameters);
#define SHOW_TASK_STACK_SIZE 128
#define SHOW_TASK_PRIORITY 2
TaskHandle_t show_task_handle;


//默认就是一般状态
Show_Mode_type show_mode = SHOW_MODE_NORMAL;

//时间设置模式的不同状态
Show_Time_Set show_time_set = TIME_SET_YEAR;
//闹钟设置模式的不同状态
Show_alarm_Set show_alarm_set = ALARM_SET_HOUR;

//表示12/24h 表示AM/PM 表示摄氏度/华氏度 闹钟12标识 充电标识 5天闹钟标识
Show_type_Struct show_type = {.is_12_hour = 0, .is_pm = 0, .is_celsius = 0, .is_alarm_1 = 0, .is_alarm_2 = 0, .is_alarm_five_day = 1, .is_charging = 1};


//开关监控触发任务
void task_switch(void *pvParameters);
#define SWITCH_TASK_STACK_SIZE 128
#define SWITCH_TASK_PRIORITY 3
TaskHandle_t switch_task_handle;

//LED灯总开关
uint8_t led_en = 1;


//闹钟结构体
Alarm_Time_Type alarm_time1 = {0};
Alarm_Time_Type alarm_time2 = {0};

// 触摸开关的计数值 => 影响屏幕亮度即可
uint8_t touch_count = 0;

//闹钟任务 => 只要是需要使用音频的任务 由于底层的驱动调用 => 必须是最高优先级
void task_alarm(void *pvParameters);
#define ALARM_TASK_STACK_SIZE 128
#define ALARM_TASK_PRIORITY 4
TaskHandle_t alarm_task_handle;

int8_t nv020d_volume = 5;
//调节音量任务 => 高优先级任务
void task_volume(void *pvParameters);
#define VOLUME_TASK_STACK_SIZE 128
#define VOLUME_TASK_PRIORITY 4
TaskHandle_t volume_task_handle;


/**
 * @brief 启动freeRTOS => 在main方法中调用 后面代码不再生效
 * 
 */
void App_freeRTOS_start(void)
{
    //1, 创建任务
    xTaskCreate(task_collect, "collect", COLLECT_TASK_STACK_SIZE, NULL, COLLECT_TASK_PRIORITY, &collect_task_handle);
    xTaskCreate(task_show, "show", SHOW_TASK_STACK_SIZE, NULL, SHOW_TASK_PRIORITY, &show_task_handle);
    xTaskCreate(task_switch, "switch", SWITCH_TASK_STACK_SIZE, NULL, SWITCH_TASK_PRIORITY, &switch_task_handle);
    xTaskCreate(task_alarm, "alarm", ALARM_TASK_STACK_SIZE, NULL, ALARM_TASK_PRIORITY, &alarm_task_handle);
    xTaskCreate(task_volume, "volume", VOLUME_TASK_STACK_SIZE, NULL, VOLUME_TASK_PRIORITY, &volume_task_handle);
    // 2, 启动freeRTOS调度器    //系统会启动一个idle空闲任务
    vTaskStartScheduler();
}

void task_collect(void *pvParameters)
{
    //1. 初始化DHT11模块 => 温湿度传感器模块
    Inf_dht11_init();
    uint8_t task_collect_count = 9; 
    while (1)
    {
        task_collect_count++;
        if (task_collect_count >= 10)
        {
            task_collect_count = 0;
            //周期性读取温湿度的值 => 不需要1s执行一次 10s执行一次
            Inf_dht_get_data(&temperature, &humidity);
        }
        // 如果进入到时间设置的模块 => 停止数据的采集
        if (show_mode != SHOW_MODE_TIME_SET)
        {
            //采集实时时钟模块的时间 => 1s执行一次
            App_dateTime_get(&clock_date_time);
            vTaskDelay(1000);
        }

        //采集实时时钟模块的时间
        App_dateTime_get(&clock_date_time);

        vTaskDelay(1000);
    }
}

void task_show(void *pvParameters)
{
    // 初始化方法
    App_show_start();
    while (1)
    {
        // 判断LED总开关
        if (led_en)
        {
            // 判断当前的状态
            if (show_mode == SHOW_MODE_NORMAL || show_mode == SHOW_MODE_VOLUME)
            {
                //展示的任务中有延时释放CPU
                APP_show_normal(&clock_date_time, temperature, humidity, &show_type);
            }
            else if (show_mode == SHOW_MODE_TIME_SET)
            {
                APP_show_time_set(&clock_date_time, temperature, humidity, &show_type);
            }
            else if (show_mode == SHOW_MODE_ALARM_SET)
            {
                App_show_alarm_set(&clock_date_time, temperature, humidity, &show_type);
            }
            //不要在展示任务的最后再添加延时了
        }
        else
        {
            //关闭所有的LED灯
            Inf_led_stop();
        }
    }

}

void task_switch(void *pvParameters)
{
    while (1)
    {
        //根据不同的模式 有不同的按键开关逻辑
        if (show_mode == SHOW_MODE_NORMAL)
        {
            //按键开关
            App_switch_key_process();
        }
        else if (show_mode == SHOW_MODE_TIME_SET)
        {
            //时间设置的按键开关
            App_switch_time_set_process();
        }
        else if (show_mode == SHOW_MODE_ALARM_SET)
        {
            //闹钟设置的按键开关
            App_switch_alarm_set_process();
        }

        //拨动开关
        App_switch_rotary_process();

        //触摸开关
        Touch_value touch_value = Inf_get_touch_value();
        if (touch_value == TOUCH)
        {
            //被触摸了
            touch_count++;
            if (touch_count >= 5)
            {
                touch_count = 0;
            }
        }
        vTaskDelay(50);
    }
    
}


void task_alarm(void *pvParameters)
{
    Inf_nv020d_init();
    uint8_t count_alarm = 0;
    uint8_t alarm_touch_count = 0;
    while (1)
    {
        //只有正常显示模块（不是在设置闹钟设置时间 才需要响闹钟）
        //如果是5天闹钟 => 判断当天是否是星期天
        if (show_type.is_alarm_five_day == 1 && clock_date_time.week >= 6)
        {
            //设置的是5天闹钟 并且是周末
        }
        else
        {
            //此处才需要响闹钟
            if (show_mode == SHOW_MODE_NORMAL)
            {
                //判断是否有闹钟
                if (show_type.is_alarm_1 == 1 )
                {
                    //判断闹钟1 => 时间储存都是24h制 闹钟保存也是24h制
                    if (alarm_time1.hour == clock_date_time.hour && alarm_time1.min == clock_date_time.min)
                    {
                        // 触发闹钟
                        if (alarm_time1.is_on == 0)
                        {
                            // 打开闹钟
                            alarm_time1.is_on = 1;
                            Inf_nv020d_send_cmd(0x00);
                            // 触发闹钟的时候 记录下当前的触摸开关状态
                            alarm_touch_count = touch_count;
                        }
                        else if (alarm_time1.is_on == 1 && alarm_time1.is_done == 0)
                        {
                            //已经在响 => 两个条件 (1)响1min (2)触摸开关
                            //(1)响1min
                            count_alarm++;
                            if (count_alarm >= 5)
                            {
                                Inf_nv020d_send_cmd(0x00);
                                count_alarm = 0;
                            }

                            // 周期性接收触发开关的值
                            if (alarm_touch_count != touch_count)
                            {
                                //触摸开关又被触摸了
                                Inf_nv020d_send_cmd(0xFE);
                                //额外记录一种状态 => 闹钟已经响过了
                                alarm_time1.is_done = 1;
                            }
                        }
                        else
                        {
                            //已经响过了
                        }
                    }
                    else
                    {
                        //关闭闹钟 已经出1min
                        if (alarm_time1.is_on == 1)
                        {
                            alarm_time1.is_on = 0;
                            alarm_time1.is_done = 0;
                            Inf_nv020d_send_cmd(0xFE);
                        }
                    }
                }
                else if (show_type.is_alarm_2 == 1)
                {
                    //判断闹钟1 => 时间储存都是24h制 闹钟保存也是24h制
                    if (alarm_time2.hour == clock_date_time.hour && alarm_time2.min == clock_date_time.min)
                    {
                        // 触发闹钟
                        if (alarm_time2.is_on == 0)
                        {
                            // 打开闹钟
                            alarm_time2.is_on = 1;
                            Inf_nv020d_send_cmd(0x00);
                            // 触发闹钟的时候 记录下当前的触摸开关状态
                            alarm_touch_count = touch_count;
                        }
                        else if (alarm_time2.is_on == 1 && alarm_time2.is_done == 0)
                        {
                            //已经在响 => 两个条件 (1)响1min (2)触摸开关
                            //(1)响1min
                            count_alarm++;
                            if (count_alarm >= 5)
                            {
                                Inf_nv020d_send_cmd(0x00);
                                count_alarm = 0;
                            }

                            // 周期性接收触发开关的值
                            if (alarm_touch_count != touch_count)
                            {
                                //触摸开关又被触摸了
                                Inf_nv020d_send_cmd(0xFE);
                                //额外记录一种状态 => 闹钟已经响过了
                                alarm_time2.is_done = 1;
                            }
                        }
                        else
                        {
                            //已经响过了
                        }
                    }
                    else
                    {
                        //关闭闹钟 已经出1min
                        if (alarm_time2.is_on == 1)
                        {
                            alarm_time2.is_on = 0;
                            alarm_time2.is_done = 0;
                            Inf_nv020d_send_cmd(0xFE);
                        }
                    }
                }
            }
        }

        

        //1s判断一次 足够及时
        vTaskDelay(1000);
    }
}


void task_volume(void *pvParameters)
{
    Inf_nv020d_init();
    // 初始化设置音量
    Inf_nv020d_send_cmd(0xE0 + nv020d_volume);
    uint8_t count_volume = 99;
    while (1)
    {
        if (show_mode == SHOW_MODE_VOLUME)
        {
            // 接受按键值 => 设置音量
            count_volume++;
            if (count_volume >= 100)
            {
                //循环播放音频
                Inf_nv020d_send_cmd(0x00);
                count_volume = 0;
            }

            //接受按键
            uint8_t key_value = Inf_get_key_value();
            switch (key_value)
            {
            case KEY_UP:
                // 增加音量
                nv020d_volume++;
                if (nv020d_volume > 7)
                {
                    nv020d_volume = 7;
                }
                Inf_nv020d_send_cmd(0xE0 + nv020d_volume);
                break;
            case KEY_DOWN:
                // 减小音量
                nv020d_volume--;
                if (nv020d_volume < 0)
                {
                    nv020d_volume = 0;
                }
                Inf_nv020d_send_cmd(0xE0 + nv020d_volume);

                break;
            case KEY_ALARM_SET:
                // 停止音量设置
                Inf_nv020d_send_cmd(0xFE);
                show_mode = SHOW_MODE_NORMAL;
                count_volume = 99;
                break;
            default:
                break;
            }
        }
        vTaskDelay(50);
    }
}
