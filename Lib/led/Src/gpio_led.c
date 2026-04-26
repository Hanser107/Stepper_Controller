#include "gpio_led.h"

static void gpio_led_on(void* self) {
    gpio_led_t *me = (gpio_led_t *)self;
    HAL_GPIO_WritePin(me->port, me->pin, GPIO_PIN_RESET);
}

static void gpio_led_off(void* self) {
    gpio_led_t *me = (gpio_led_t *)self;
    HAL_GPIO_WritePin(me->port, me->pin, GPIO_PIN_SET);
}

static void gpio_led_toggle(void* self) {
    gpio_led_t *me = (gpio_led_t *)self;
    HAL_GPIO_TogglePin(me->port, me->pin);
}

static const struct led_ops gpio_led_ops = {
    .on = gpio_led_on,
    .off = gpio_led_off,
    .toggle = gpio_led_toggle,
};

// 构造函数
void gpio_led_init(gpio_led_t *led, GPIO_TypeDef *port, uint16_t pin) {
    led->base.ops = &gpio_led_ops;   // 绑定虚表
    led->port = port;
    led->pin  = pin;
    // 硬件初始化
    // GPIO_InitTypeDef GPIO_InitStruct = {0};
    // GPIO_InitStruct.Pin   = pin;
    // GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    //
    // HAL_GPIO_Init(port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET); // 默认熄灭
}
