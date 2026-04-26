#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>

// 按键事件类型枚举
typedef enum {
    BUTTON_EVENT_PRESS,      // 按下（电平变化瞬间）
    BUTTON_EVENT_RELEASE,    // 释放
    BUTTON_EVENT_CLICK,      // 单击（按下+释放）
    BUTTON_EVENT_LONG_PRESS  // 长按
} button_event_t;

// 按键操作虚表
struct button_ops {
    bool (*is_pressed)(void *self);               // 查询当前是否按下
    void (*update)(void *self);                   // 状态机更新（需周期性调用）
    bool (*get_event)(void *self, button_event_t *evt); // 获取并清除事件
};

// 按键基类
typedef struct {
    const struct button_ops *ops;
} button_t;

// 统一操作接口（内联函数）
static inline bool button_is_pressed(button_t *btn) {
    return btn->ops->is_pressed(btn);
}

static inline void button_update(button_t *btn) {
    btn->ops->update(btn);
}

static inline bool button_get_event(button_t *btn, button_event_t *evt) {
    return btn->ops->get_event(btn, evt);
}

#endif