#ifndef GPIO_BUTTON_H
#define GPIO_BUTTON_H

#include "button.h"
#include "stm32f1xx_hal.h"   // 示例使用 HAL 库

// GPIO 按键派生类（公开结构体，便于静态分配）
typedef struct {
    button_t base;                // 继承基类
    GPIO_TypeDef *port;            // GPIO 端口
    uint16_t pin;                  // 引脚
    uint8_t active_level;          // 有效电平（0：低有效，1：高有效）

    // 状态机内部变量（用户只读不写）
    uint8_t state;                 // 当前去抖状态
    uint32_t last_time;            // 上次状态变化时间戳
    bool event_press;              // 按下事件标志
    bool event_release;            // 释放事件标志
    bool event_click;              // 单击事件标志
    bool event_long;               // 长按事件标志
    uint32_t press_start_time;     // 按下时刻时间戳
} gpio_button_t;

// 构造函数
void gpio_button_init(gpio_button_t *btn, GPIO_TypeDef *port, uint16_t pin, uint8_t active_level);

// 注意：需要由用户周期性提供系统时间戳（毫秒）
// 典型用法：在定时器中断或主循环中调用 button_update() 前，先设置全局 tick

#endif