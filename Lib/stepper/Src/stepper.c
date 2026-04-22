#include "stepper.h"
#include "stepper_hal.h"
#include <stdlib.h>
#include <string.h>

#define SPEED_TABLE_SIZE 3200

typedef struct {
    uint32_t v_start;
    uint32_t v_const;
    uint32_t v_end;

    uint32_t acc_step;
    uint32_t dec_step;

    uint32_t idx;
    uint32_t speed_table[SPEED_TABLE_SIZE];
}SCurve_Handle_t;

/* 电机实例内部结构（隐藏实现） */
struct StepperMotor {
    /* 硬件资源句柄 */
    StepperTimer *timer;
    StepperGpio *dir_gpio;
    StepperGpio *en_gpio;

    /* 用户配置 */
    uint32_t steps_per_rev;     /* 每转步数 */
    bool dir_active_level;      /* 电机正转时的电平 */
    bool en_active_level;       /* 使能有效时的电平 */

    /* 状态变量 */
    volatile MotorState state;
    volatile int32_t total_run_steps;   /* 总运动步数 */
    uint32_t target_steps;      /* 目标步数 */
    uint32_t current_steps;     /* 当前步数 */
    uint8_t current_dir;        /* 当前方向 */
    uint8_t is_continuous;      /* 持续运行 */

    /* 速度变量结构体 */
    SCurve_Handle_t s_curve;

    /* 脉冲单周期中断回调函数 */
    void *one_period_callback;
    /* 脉冲完成回调参数 */
    void *callback_arg;
};

/* 脉冲完成时的内部回调（由HAL层调用） */
// static void on_pulse_done_callback(void *arg) {
//     StepperMotor *motor = (StepperMotor*)arg;
//     motor->state = MOTOR_IDLE;
//     motor->target_steps = 0;
//     motor->current_steps = 0;
// }

void one_period_done_callback(StepperMotor *motor) {
    if (motor->state == MOTOR_IDLE) return;
    if (motor->current_steps < motor->target_steps) {
        motor->current_steps++;
    }
    else {
        motor->state = MOTOR_IDLE;
        motor->target_steps = 0;
        motor->current_steps = 0;
        stepper_hal->timer_stop_pulse(motor->timer);
    }
}

/* 创建电机实例 */
StepperMotor* stepper_create(const StepperConfig *cfg) {
    if (!cfg || !cfg->htim) return NULL;

    StepperMotor *motor = (StepperMotor*)malloc(sizeof(StepperMotor));
    if (!motor) return NULL;

    memset(motor, 0, sizeof(StepperMotor));

    /* 保存用户参数 */
    motor->steps_per_rev = cfg->steps_per_rev;
    motor->dir_active_level = cfg->dir_active_level;
    motor->en_active_level = cfg->en_active_level;

    /* 初始化方向 GPIO */
    StepperGpioConfig dir_cfg = {
        .port = cfg->dir_port,
        .pin = cfg->dir_pin,
        .init_level = !motor->dir_active_level   /* 初始设为反向有效电平，避免误动 */
    };
    motor->dir_gpio = stepper_hal->gpio_init(&dir_cfg);
    if (!motor->dir_gpio) {
        free(motor);
        return NULL;
    }
    /* 初始化使能 GPIO */
    if (cfg->en_port) {
        StepperGpioConfig en_cfg = {
            .port = cfg->en_port,
            .pin = cfg->en_pin,
            .init_level = motor->en_active_level   /* 初始使能 */
        };
        motor->en_gpio = stepper_hal->gpio_init(&en_cfg);
        if (!motor->en_gpio) {
            free(motor);
            return NULL;
        }
        /* 默认使能电机 */
        stepper_hal->gpio_write(motor->en_gpio, motor->en_active_level);
    } else {
        motor->en_gpio = NULL;
    }

    /* 初始化定时器 */
    StepperTimerConfig timer_cfg = {
        .htim = cfg->htim,
        .channel = cfg->timer_channel,
        .period_us = 0   /* 暂不设置，启动时更新 */
    };
    motor->timer = stepper_hal->timer_init(&timer_cfg);
    if (!motor->timer) {
        if (motor->dir_gpio) stepper_hal->gpio_deinit(motor->dir_gpio);
        if (motor->en_gpio) stepper_hal->gpio_deinit(motor->en_gpio);
        if (motor->timer) stepper_hal->timer_deinit(motor->timer);
        free(motor);
        return NULL;

    }
    else {
        motor->state = MOTOR_IDLE;
        motor->target_steps = 0;
        motor->current_steps = 0;
        motor->total_run_steps = 0;
        motor->one_period_callback = one_period_done_callback;
        return motor;
    }
}

/* 销毁电机实例 */
void stepper_destroy(StepperMotor *motor) {
    if (!motor) return;
    stepper_stop(motor);
    if (motor->timer) stepper_hal->timer_deinit(motor->timer);
    if (motor->dir_gpio) stepper_hal->gpio_deinit(motor->dir_gpio);
    if (motor->en_gpio) stepper_hal->gpio_deinit(motor->en_gpio);
    free(motor);
}

/* 使能/失能电机 */
void stepper_enable(StepperMotor *motor, bool enable) {
    if (!motor || !motor->en_gpio) return;
    stepper_hal->gpio_write(motor->en_gpio, enable ? motor->en_active_level : !motor->en_active_level);
}

/* 停止运动(急停) */
void stepper_stop(StepperMotor *motor) {
    if (!motor) return;
    if (motor->timer) {
        stepper_hal->timer_stop_pulse(motor->timer);
    }
    motor->state = MOTOR_STOP;
    motor->target_steps = 0;
    motor->current_steps = 0;
}

/* 检查电机状态 */
MotorState stepper_is_idle(StepperMotor *motor) {
    if (!motor) return true;
    return motor->state;
}


void stepper_test_run(StepperMotor *motor, uint32_t dir) {
    if (!motor) return;

    stepper_hal->gpio_write(motor->dir_gpio, dir ? motor->dir_active_level : !motor->dir_active_level);
    stepper_hal->timer_set_freq(motor->timer, 500);
    stepper_hal->timer_start_pulse(motor->timer);

    motor->current_dir = dir;
}



