#ifndef STEPPER_HAL_H
#define STEPPER_HAL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct StepperTimer StepperTimer;
typedef struct StepperGpio StepperGpio;

/* 定时器配置结构体 */
typedef struct {
    void *htim;             /* TIM_HandleTypeDef* */
    uint32_t channel;       /* TIM_CHANNEL_1..4 */
    uint32_t period_us;     /* 脉冲周期（微秒），由上层根据频率计算 */
} StepperTimerConfig;

/* GPIO 配置结构体 */
typedef struct {
    void *port;             /* GPIO_TypeDef* */
    uint16_t pin;
    bool init_level;        /* 初始电平 */
} StepperGpioConfig;

/* 硬件抽象层 API 结构体 */
typedef struct {
    /* 定时器操作 */
    StepperTimer* (*timer_init)(const StepperTimerConfig *cfg);
    void (*timer_start_pulse)(StepperTimer *timer);
    void (*timer_set_freq)(StepperTimer *timer, uint32_t freq);
    void (*timer_stop_pulse)(StepperTimer *timer);
    void (*timer_deinit)(StepperTimer *timer);

    /* GPIO 操作 */
    StepperGpio* (*gpio_init)(const StepperGpioConfig *cfg);
    void (*gpio_write)(StepperGpio *gpio, bool level);
    void (*gpio_deinit)(StepperGpio *gpio);
} StepperHalApi;

/* 全局 HAL API 指针（由平台实现文件赋值） */
extern const StepperHalApi *stepper_hal;

#endif