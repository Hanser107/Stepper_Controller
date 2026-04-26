#ifndef INC_GPIO_LED_H_
#define INC_GPIO_LED_H_

#include "led.h"
#include "stm32f1xx_hal.h"

// GPIO LED 派生结构体
typedef struct {
    led_base base;
    GPIO_TypeDef *port;
    uint16_t pin;
} gpio_led_t;

void gpio_led_init(gpio_led_t *led, GPIO_TypeDef *port, uint16_t pin);


#endif
