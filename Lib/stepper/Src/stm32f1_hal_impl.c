#include "stepper_hal.h"
#include "stm32f1xx_hal.h"
#include "string.h"
#include "tim.h"

/* 定时器句柄扩展结构（包含HAL定时器句柄和通道） */
struct StepperTimer {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t timer_prescaler;

    bool is_running;
};

/* GPIO 句柄扩展结构 */
struct StepperGpio {
    GPIO_TypeDef *port;
    uint16_t pin;
};

/* 静态分配（实际项目可用动态内存，这里简单用全局数组，支持最多4个定时器） */
#define MAX_TIMERS 4
static StepperTimer timers[MAX_TIMERS];
static uint8_t timer_used[MAX_TIMERS] = {0};

#define MAX_GPIOS 8
static StepperGpio gpios[MAX_GPIOS];
static uint8_t gpio_used[MAX_GPIOS] = {0};

/* 查找空闲定时器槽位 */
static StepperTimer* alloc_timer(void) {
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (!timer_used[i]) {
            timer_used[i] = 1;
            memset(&timers[i], 0, sizeof(StepperTimer));
            return &timers[i];
        }
    }
    return NULL;
}

static void free_timer(StepperTimer *timer) {
    if (timer) {
        int idx = timer - timers;
        if (idx >= 0 && idx < MAX_TIMERS) {
            timer_used[idx] = 0;
        }
    }
}

/* 查找空闲GPIO槽位 */
static StepperGpio* alloc_gpio(void) {
    for (int i = 0; i < MAX_GPIOS; i++) {
        if (!gpio_used[i]) {
            gpio_used[i] = 1;
            memset(&gpios[i], 0, sizeof(StepperGpio));
            return &gpios[i];
        }
    }
    return NULL;
}

static void free_gpio(StepperGpio *gpio) {
    if (gpio) {
        int idx = gpio - gpios;
        if (idx >= 0 && idx < MAX_GPIOS) {
            gpio_used[idx] = 0;
        }
    }
}

/* 定时器中断回调（需要在 stm32f4xx_it.c 中调用此函数） */
void Stepper_HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim) {

}

/* ---------- 实现 HAL 接口函数 ---------- */
static StepperTimer* timer_init_impl(const StepperTimerConfig *cfg) {
    if (!cfg || !cfg->htim) return NULL;
    StepperTimer *timer = alloc_timer();
    if (!timer) return NULL;

    timer->htim = (TIM_HandleTypeDef*)cfg->htim;
    timer->channel = cfg->channel;
    timer->is_running = false;
    return timer;
}

static void timer_start_pulse_impl(StepperTimer *timer) {
    if (!timer || !timer->htim) return;

    /* 停止当前运行 */
    if (timer->is_running) {
        HAL_TIM_Base_Stop_IT(timer->htim);
        HAL_TIM_PWM_Stop_IT(timer->htim, timer->channel);
        timer->is_running = false;
    }
    HAL_TIM_Base_Start_IT(timer->htim);
    HAL_TIM_PWM_Start_IT(timer->htim, timer->channel);


    timer->is_running = true;

}

static void timer_set_freq_impl(StepperTimer *timer, uint32_t freq) {
    if (!timer) return;
    freq = 200;
    __HAL_TIM_SET_AUTORELOAD(timer->htim, freq);
}

static void timer_stop_pulse_impl(StepperTimer *timer) {
    if (!timer || !timer->is_running) return;
    HAL_TIM_OC_Stop_IT(timer->htim, timer->channel);
    timer->is_running = false;
}

static void timer_deinit_impl(StepperTimer *timer) {
    if (!timer) return;
    if (timer->is_running) {
        HAL_TIM_OC_Stop_IT(timer->htim, timer->channel);
    }
    free_timer(timer);
}

/* GPIO 实现 */
static StepperGpio* gpio_init_impl(const StepperGpioConfig *cfg) {
    if (!cfg || !cfg->port) return NULL;
    StepperGpio *gpio = alloc_gpio();
    if (!gpio) return NULL;

    gpio->port = (GPIO_TypeDef*)cfg->port;
    gpio->pin = cfg->pin;

    /* 配置 GPIO 为输出模式（需要 HAL 库初始化，假设已在 CubeMX 中初始化） */
    /* 这里简单设置初始电平 */
    HAL_GPIO_WritePin(gpio->port, gpio->pin, cfg->init_level ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return gpio;
}

static void gpio_write_impl(StepperGpio *gpio, bool level) {
    if (!gpio) return;
    HAL_GPIO_WritePin(gpio->port, gpio->pin, level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void gpio_deinit_impl(StepperGpio *gpio) {
    if (!gpio) return;
    /* 可选：将引脚配置为默认状态 */
    free_gpio(gpio);
}

/* 全局 HAL API 结构体实例 */
static const StepperHalApi stm32f4_hal_api = {
    .timer_init = timer_init_impl,
    .timer_start_pulse = timer_start_pulse_impl,
    .timer_set_freq = timer_set_freq_impl,
    .timer_stop_pulse = timer_stop_pulse_impl,
    .timer_deinit = timer_deinit_impl,

    .gpio_init = gpio_init_impl,
    .gpio_write = gpio_write_impl,
    .gpio_deinit = gpio_deinit_impl
};

const StepperHalApi *stepper_hal = &stm32f4_hal_api;

