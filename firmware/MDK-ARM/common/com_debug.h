#ifndef __COM_DEBUG_H
#define __COM_DEBUG_H

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"


// 1. 设置日志打印开关 实际开发环境 日志打印输出比较占用CPU资源，调试完成后建议关闭

//2. 添加前缀 补全文件名称和行号，方便定位

#define DEBUG_ENABLE 1
#ifdef DEBUG_ENABLE

// 统一使用通用的宏去做日志输出
#define debug_printf(format,...) printf( "[%s:%d]" format "\r\n", __FILE__, __LINE__, ##__VA_ARGS__)

// 如果关闭日志打印开关
#else
#define debug_printf(...)

#endif // DEBUG_ENABLE


#endif // !__COM_DEBUG_H
