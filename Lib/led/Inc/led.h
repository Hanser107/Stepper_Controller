#ifndef INC_LED_H
#define INC_LED_H

struct led_ops {
    void (*on)(void *);
    void (*off)(void *);
    void (*toggle)(void *);
};

/* LED基类结构体 */
typedef struct {
    const struct led_ops *ops;   // 虚表指针
} led_base;

static inline __attribute__((unused)) void led_on(led_base *base) {
    base->ops->on(base);
}

static inline __attribute__((unused)) void led_off(led_base *base) {
    base->ops->off(base);
}

static inline __attribute__((unused)) void led_toggle(led_base *base) {
    base->ops->toggle(base);
}

#endif
