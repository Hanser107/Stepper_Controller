#include "gpio_button.h"

// 去抖时间（毫秒）
#define DEBOUNCE_MS     20
// 长按判定时间（毫秒）
#define LONG_PRESS_MS   1000

// 外部全局系统 tick（由用户提供，例如 SysTick 中断递增）
extern volatile uint32_t system_tick_ms;

/* ----- 私有方法实现 ----- */
static bool gpio_button_is_pressed(void *self) {
    gpio_button_t *me = (gpio_button_t*)self;
    uint8_t level = (me->active_level == 0) ? 0 : 1;
    return (HAL_GPIO_ReadPin(me->port, me->pin) == level);
}

static void gpio_button_update(void *self) {
    gpio_button_t *me = (gpio_button_t*)self;
    uint32_t now = system_tick_ms;

    // 读取当前物理电平（未去抖）
    bool raw_pressed = gpio_button_is_pressed(me);
    //bool stable_pressed = (me->state == 1);

    // 清除单次事件标志
    me->event_press  = false;
    me->event_release = false;
    me->event_click  = false;
    me->event_long   = false;

    // 简单去抖状态机
    switch (me->state) {
        case 0: // 稳定释放状态
            if (raw_pressed) {
                me->state = 1;
                me->last_time = now;
            }
            break;
        case 1: // 抖动/确认中
            if (!raw_pressed) {
                me->state = 0; // 抖动，回退
            } else if (now - me->last_time >= DEBOUNCE_MS) {
                me->state = 2;
                me->press_start_time = now;
                me->event_press = true;      // 按下事件
            }
            break;
        case 2: // 稳定按下状态
            if (!raw_pressed) {
                me->state = 3;
                me->last_time = now;
            } else {
                // 检查长按
                if (!me->event_long && (now - me->press_start_time >= LONG_PRESS_MS)) {
                    me->event_long = true;   // 长按事件
                }
            }
            break;
        case 3: // 释放抖动/确认中
            if (raw_pressed) {
                me->state = 2; // 抖动，回退
            } else if (now - me->last_time >= DEBOUNCE_MS) {
                me->state = 0;
                me->event_release = true;    // 释放事件
                // 若未触发过长按，则产生单击事件
                if (!me->event_long) {
                    me->event_click = true;
                }
            }
        default:
            break;
    }
}


static bool gpio_button_get_event(void *self, button_event_t *evt) {
    gpio_button_t *me = (gpio_button_t*)self;
    if (me->event_press) {
        if (evt) *evt = BUTTON_EVENT_PRESS;
        return true;
    }
    if (me->event_release) {
        if (evt) *evt = BUTTON_EVENT_RELEASE;
        return true;
    }
    if (me->event_click) {
        if (evt) *evt = BUTTON_EVENT_CLICK;
        return true;
    }
    if (me->event_long) {
        if (evt) *evt = BUTTON_EVENT_LONG_PRESS;
        return true;
    }
    return false;
}

// 虚表实例
static const struct button_ops gpio_button_ops = {
    .is_pressed = gpio_button_is_pressed,
    .update     = gpio_button_update,
    .get_event  = gpio_button_get_event
};

/* ----- 构造函数 ----- */
void gpio_button_init(gpio_button_t *btn, GPIO_TypeDef *port, uint16_t pin, uint8_t active_level) {
    btn->base.ops = &gpio_button_ops;
    btn->port = port;
    btn->pin  = pin;
    btn->active_level = active_level;
    btn->state = 0;
    btn->last_time = 0;
    btn->event_press = false;
    btn->event_release = false;
    btn->event_click = false;
    btn->event_long = false;
    btn->press_start_time = 0;

    // 硬件初始化：配置为上拉输入（若有效电平为低）或下拉输入（若有效电平为高）
    // GPIO_InitTypeDef GPIO_InitStruct = {0};
    // GPIO_InitStruct.Pin = pin;
    // GPIO_InitStruct.Mode = (active_level == 0) ? GPIO_MODE_INPUT : GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = (active_level == 0) ? GPIO_PULLUP : GPIO_PULLDOWN;
    // HAL_GPIO_Init(port, &GPIO_InitStruct);
}