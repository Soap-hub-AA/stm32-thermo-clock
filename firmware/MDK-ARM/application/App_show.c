#include "App_show.h"


//时钟的点需要1s闪一次
uint8_t clock_flag = 0;
uint32_t clock_last_time = 0;

extern Show_Time_Set show_time_set;

// 获取show_alalrm_set
extern Show_alarm_Set show_alarm_set;

// 获取闹钟时间结构体
extern Alarm_Time_Type alarm_time1;
extern Alarm_Time_Type alarm_time2;
/**
 * @brief 初始化 开启LED灯
 * 
 */
void App_show_start(void)
{
    Inf_led_start();
    //最开始的时间值 => 用于后续1s的判断
    clock_last_time = xTaskGetTickCount();
}


/**
 * @brief 对应一般模式的LED显示
 * 时钟的点需要1s闪一次
 * 
 */
void APP_show_normal(Clock_Date_time_type *clock_date_time, int8_t temperature, int8_t humidity, Show_type_Struct *show_type)
{
    //1.  展示时间 => 两个点1s闪一次
    // Inf_led_set_clock(clock_date_time->hour, clock_date_time->min, clock_flag);

    // 每过1s => 500ms切换flag状态 => 500ms时间（以5ms一次闪烁）  500ms根本不亮
    if (xTaskGetTickCount() - clock_last_time >= 500)
    {
        clock_flag = (clock_flag + 1) % 2;
        clock_last_time = xTaskGetTickCount();
    }

    //要根据展示的类型 修正时间和温度的值 => 不同的显示类型和单位
    //1.修正小时的值 
    uint8_t hour = 0;
    //if (show_type->is_12_hour)
    // {
    //     if (show_type->is_pm)
    //     {
    //         hour = clock_date_time->hour + 12;
    //     }
    //     else
    //     {
    //         hour = clock_date_time->hour;
    //     }
    // }
    // else
    // {
    //     hour = clock_date_time->hour;
    // }
    if (clock_date_time->is_12_hour && clock_date_time->is_pm)
    {
        hour = 12 + clock_date_time->hour;
    }
    else
    {
        hour = clock_date_time->hour;
    }

    //2.判断当前展示的是什么类型
    if (show_type->is_12_hour)
    {
        if (hour >= 12)
        {
            hour = hour - 12;
            show_type->is_pm = 1;
        }
        else
        {
            show_type->is_pm = 0;
        }
    }


    //展示所有内容
    Inf_led_set_all(hour, clock_date_time->min, clock_flag, temperature, humidity, show_type);
}

/**
 * @brief 对应时间设置模式的LED显示
 * 
 * @param clock_date_time 时钟的日期时间结构体
 */
void APP_show_time_set(Clock_Date_time_type *clock_date_time, int8_t temperature, int8_t humidity, Show_type_Struct *show_type)
{
    
    if (xTaskGetTickCount() - clock_last_time >= 500)
    {
        clock_flag = (clock_flag + 1) % 2;
        clock_last_time = xTaskGetTickCount();
    }
    //判断当前处于不同的时间按设置页面 => 调用Inf_led_set_time_set内容
    //1.判断当前处于的页面
    //后续正常显示温湿度页面
    switch (show_time_set)
    {
    case TIME_SET_YEAR:
        // Inf_led_time_set(TIME_SET_YEAR,clock_date_time->year);
        if (clock_flag)
        {
            //亮
            Inf_led_set_all(clock_date_time->year / 100, clock_date_time->year % 100, 0, temperature, humidity, show_type);
        }
        else
        {
            //后面两个年份的数字不亮
            Inf_led_set_all(clock_date_time->year / 100, 0xff, 0, temperature, humidity, show_type);
        }
        break;
    case TIME_SET_MONTH:
        // Inf_led_time_set(TIME_SET_MONTH,clock_date_time->month);
        if (clock_flag)
        {
            //亮
            Inf_led_set_all(clock_date_time->month, 0xff, 0, temperature, humidity, show_type);
        }
        else
        {
            Inf_led_set_all(0xff, 0xff, 0, temperature, humidity, show_type);
        }
        break;
    case TIME_SET_DAY:
        // Inf_led_time_set(TIME_SET_DAY,clock_date_time->day);
        if (clock_flag)
        {
            Inf_led_set_all(0xff, clock_date_time->day, 0, temperature, humidity, show_type);
        }
        else
        {
            Inf_led_set_all(0xff, 0xff, 0, temperature, humidity, show_type);
        }
        break;
    case TIME_SET_HOUR:
        // Inf_led_time_set(TIME_SET_HOUR,clock_date_time->hour);
        if (clock_flag)
        {
            Inf_led_set_all(clock_date_time->hour, 0xff, 1, temperature, humidity, show_type);
        }
        else
        {
            Inf_led_set_all(0xff, 0xff, 1, temperature, humidity, show_type);
        }
        break;
    case TIME_SET_MIN:
        // Inf_led_time_set(TIME_SET_MIN,clock_date_time->min);
        if (clock_flag)
        {
            Inf_led_set_all(0xff, clock_date_time->min, 1, temperature, humidity, show_type);
        }
        else
        {
            Inf_led_set_all(0xff, 0xff, 1, temperature, humidity, show_type);
        }
        break;
    default:
        break;
    }
}



/**
 * @brief 对应闹钟设置模式的LED显示
 * 
 * 
 */
void App_show_alarm_set(Clock_Date_time_type *clock_date_time, int8_t temperature, int8_t humidity, Show_type_Struct *show_type)
{
    if (xTaskGetTickCount() - clock_last_time >= 500)
    {
        clock_flag = (clock_flag + 1) % 2;
        clock_last_time = xTaskGetTickCount();
    }
    switch (show_alarm_set)
    {
    case ALARM_SET_HOUR:
        if (clock_flag)
        {
            // 亮 => 展示对应闹钟时间的小时
            if (show_type->is_alarm_1)
            {
                Inf_led_set_all(alarm_time1.hour, 0xff, 1, temperature, humidity, show_type);
            }
            else if (show_type->is_alarm_2)
            {
                Inf_led_set_all(alarm_time2.hour, 0xff, 1, temperature, humidity, show_type);
            }
        }
        else
        {
            // 不亮
            Inf_led_set_all(0xff, 0xff, 1, temperature, humidity, show_type);
        }

        break;
    case ALARM_SET_MIN:
        // 判断亮
        if (clock_flag)
        {
            // 判断具体是哪个闹钟
            if (show_type->is_alarm_1)
            {
                Inf_led_set_all(0xff, alarm_time1.min, 1, temperature, humidity, show_type);
            }
            else if (show_type->is_alarm_2)
            {
                Inf_led_set_all(0xff, alarm_time2.min, 1, temperature, humidity, show_type);
            }
        }
        else
        {
            Inf_led_set_all(0xff, 0xff, 1, temperature, humidity, show_type);
        }

        break;
    default:
        break;
    }

}