#include "App_dateTime.h"



/**
 * @brief 读取所有的时间
 * 
 */
void App_dateTime_get(Clock_Date_time_type *date_time)
{
    //1. 读取年份
    uint8_t year_data = Inf_DS1302Z_Read_Byte(DS1302Z_YEAR_REG);
    //只包含年份的后两位
    date_time->year = (year_data >> 4)*10 + (year_data & 0x0f) + 2000;

    //2. 读取月份
    uint8_t month_data = Inf_DS1302Z_Read_Byte(DS1302Z_MONTH_REG);
    date_time->month = (month_data >> 4)*10 + (month_data & 0x0f);

    //3. 读取日期
    uint8_t day_data = Inf_DS1302Z_Read_Byte(DS1302Z_DAY_REG);
    date_time->day = (day_data >> 4)*10 + (day_data & 0x0f);

    //4. 读取星期
    uint8_t week_data = Inf_DS1302Z_Read_Byte(DS1302Z_WEEK_REG);
    date_time->week = (week_data & 0x0f);

    //5. 读取小时
    uint8_t hour_data = Inf_DS1302Z_Read_Byte(DS1302Z_HOUR_REG);
    if (hour_data & 0x80)
    {
        //判断使用的是12小时
        date_time->is_12_hour = 1;
        if (hour_data & 0x20)
        {
            date_time->is_pm = 1;
        }
        else
        {
            date_time->is_pm = 0;
        }

        if (hour_data & 0x10)
        {
            date_time->hour = 10 + (hour_data & 0x0f);
        }
        else
        {
            date_time->hour = (hour_data & 0x0f);
        }
    }
    else
    {
        //24小时
        date_time->is_12_hour = 0;
        if (hour_data & 0x20)
        {
            //20小时位
            date_time->hour = 20 + (hour_data & 0x0f);
        }
        else if (hour_data & 0x10)
        {
            //10小时位
            date_time->hour = 10 + (hour_data & 0x0f);
        }
        else
        {
            date_time->hour = (hour_data & 0x0f);
        }
    }

    // 统一使用24h制保存日期
    if (date_time->is_12_hour == 1)
    {
        if (date_time->is_pm)
        {
            date_time->hour = date_time->hour + 12;
        }
        //统一切换为24h制保存日期
        date_time->is_12_hour = 0;
    }
    
    //6. 读取分钟
    uint8_t minute_data = Inf_DS1302Z_Read_Byte(DS1302Z_MINUTE_REG);
    date_time->min = (minute_data >> 4)*10 + (minute_data & 0x0f);

}

void App_dateTime_set(Clock_Date_time_type *date_time)
{
    //将date_time中的时间写入到芯片中
    //0.将写保护寄存器写为0
    Inf_DS1302Z_Write_Byte(DS1302Z_CONTROL_REG, 0X00);
    //1. 写入年份
    uint8_t year_shi = (date_time->year % 100) / 10;
    uint8_t year_ge = date_time->year % 10;
    Inf_DS1302Z_Write_Byte(DS1302Z_YEAR_REG, (year_shi << 4) | year_ge);

    //2. 写入月份
    uint8_t month_shi = date_time->month / 10;
    uint8_t month_ge = date_time->month % 10;
    Inf_DS1302Z_Write_Byte(DS1302Z_MONTH_REG, (((year_shi & 0x01) << 4) | month_ge));

    //3. 写入日
    uint8_t day_shi = date_time->day / 10;
    uint8_t day_ge = date_time->day % 10;
    Inf_DS1302Z_Write_Byte(DS1302Z_DAY_REG, (((day_shi & 0x03) << 4) | day_ge));

    //4. 写入小时 => 时间设置都是使用24小时制
    if (date_time->hour > 12)
    {
        //下午时间 
        uint8_t hour_shi = date_time->hour / 10;
        uint8_t hour_ge = date_time->hour % 10;
        Inf_DS1302Z_Write_Byte(DS1302Z_HOUR_REG, (0x80 | (0x20) |(hour_shi << 4)|(hour_ge % 10)));
    }
    else
    {
        uint8_t hour_shi = date_time->hour / 10;
        uint8_t hour_ge = date_time->hour % 10;
        Inf_DS1302Z_Write_Byte(DS1302Z_HOUR_REG, (0x80 | (hour_shi << 4) | (hour_ge % 10)));
    }

    //5. 写入分钟
    uint8_t min_shi = date_time->min / 10;
    uint8_t min_ge = date_time->min % 10;
    Inf_DS1302Z_Write_Byte(DS1302Z_MINUTE_REG, ((min_shi & 0x07) << 4) | (min_ge % 10));

    //6. 秒不需要写 => 默认0
    Inf_DS1302Z_Write_Byte(DS1302Z_SECOND_REG, 0x00);

    //7. 计算当前为星期几的蔡勒公式
    uint16_t y = date_time->year;
    uint8_t c = (y / 100) + 1;
    uint8_t m = date_time->month;
    if (m == 1 || m == 2)
    {
        y--;
        m += 12;
    }
    //蔡勒公式
    int16_t w = y + y / 4 + c / 4 - 2 * c + 26 * (m + 1) / 10 + date_time->day - 1;
    while (w < 0)
    {
        w += 7;
    }
    w = w % 7;
    if (w == 0)
    {
        w = 7;
    }

    Inf_DS1302Z_Write_Byte(DS1302Z_WEEK_REG, w);
}
