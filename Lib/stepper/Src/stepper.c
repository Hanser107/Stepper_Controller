#include "stepper.h"
#include "stepper_hal.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    uint32_t v_start;
    uint32_t v_const;
    uint32_t v_end;

    uint32_t acc_step;
    uint32_t const_step;
    uint32_t dec_step;

    uint32_t idx;
    uint32_t speed_table[SPEED_TABLE_SIZE];
}scurve_Handle_t;

/* 电机实例内部结构（隐藏实现） */
struct stepper_motor_t {
    /* 硬件资源句柄 */
    StepperTimer *timer;
    StepperGpio *dir_gpio;
    StepperGpio *en_gpio;

    /* 用户配置 */
    uint32_t steps_per_rev;     /* 每转步数 */
    uint8_t dir_active_level;      /* 电机正转时的电平 */
    uint8_t en_active_level;       /* 使能有效时的电平 */

    /* 状态变量 */
    volatile MotorState state;
    volatile int32_t total_run_steps;   /* 总运动步数 */
    uint32_t target_steps;      /* 目标步数 */
    uint32_t current_steps;     /* 当前步数 */
    Rotation_Dir current_dir;        /* 当前方向 */

    uint8_t is_continuous;      /* 持续运行 */
    uint8_t exit_continuous;

    /* 速度变量结构体 */
    scurve_Handle_t * s_curve;

    /* 脉冲单周期中断回调函数 */
    void *one_period_callback;
    /* 脉冲完成回调参数 */
    void *callback_arg;
};

scurve_Handle_t* SCurve_InitCreate(void) {
    scurve_Handle_t* s_curve = (scurve_Handle_t*)malloc(sizeof(scurve_Handle_t));
    if (!s_curve) return NULL;
    s_curve->v_start = STEPPER_MIN_SPEED;
    s_curve->v_const = STEPPER_MIN_SPEED;
    s_curve->v_end = STEPPER_MIN_SPEED;

    s_curve->acc_step = 0;
    s_curve->const_step = 0;
    s_curve->dec_step = 0;
    s_curve->idx = 0;

    memset(s_curve->speed_table, 0, sizeof(s_curve->speed_table));

    return s_curve;
}

void SCurve_destroy(scurve_Handle_t* scurve) {
    if (!scurve) return;
    free(scurve);
}

/* 创建电机实例 */
stepper_motor_t* stepper_create(const StepperConfig *cfg) {
    if (!cfg || !cfg->htim) return NULL;

    stepper_motor_t *motor = (stepper_motor_t*)malloc(sizeof(stepper_motor_t));
    if (!motor) return NULL;

    memset(motor, 0, sizeof(stepper_motor_t));

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
        scurve_Handle_t* s_curve = SCurve_InitCreate();

        motor->state = MOTOR_IDLE;
        motor->is_continuous = false;
        motor->exit_continuous = 0;
        motor->target_steps = 0;
        motor->current_steps = 0;
        motor->total_run_steps = 0;
        if (!s_curve) {
            if (motor->dir_gpio) stepper_hal->gpio_deinit(motor->dir_gpio);
            if (motor->en_gpio) stepper_hal->gpio_deinit(motor->en_gpio);
            if (motor->timer) stepper_hal->timer_deinit(motor->timer);
            free(motor);
            SCurve_destroy(s_curve);
            return NULL;
        }
        motor->s_curve = s_curve;
        motor->one_period_callback = one_period_done_callback;
        return motor;
    }
}

/* 销毁电机实例 */
void stepper_destroy(stepper_motor_t *motor) {
    if (!motor) return;
    stepper_stop(motor);
    if (motor->timer) stepper_hal->timer_deinit(motor->timer);
    if (motor->dir_gpio) stepper_hal->gpio_deinit(motor->dir_gpio);
    if (motor->en_gpio) stepper_hal->gpio_deinit(motor->en_gpio);
    free(motor);
}

/* 使能/失能电机 */
void stepper_enable(stepper_motor_t *motor, uint8_t enable) {
    if (!motor || !motor->en_gpio) return;
    stepper_hal->gpio_write(motor->en_gpio, enable ? motor->en_active_level : !motor->en_active_level);
}

/* 停止运动(急停) */
void stepper_stop(stepper_motor_t *motor) {
    if (!motor) return;
    if (motor->timer) {
        stepper_hal->timer_stop_pulse(motor->timer);
    }
    motor->state = MOTOR_STOP;
    motor->target_steps = 0;
    motor->current_steps = 0;
}

/* 检查电机状态 */
MotorState stepper_is_idle(stepper_motor_t *motor) {
    if (!motor) return 0;
    return motor->state;
}


void stepper_test_run(stepper_motor_t *motor, uint32_t dir) {
    if (!motor) return;

    stepper_hal->gpio_write(motor->dir_gpio, dir ? motor->dir_active_level : !motor->dir_active_level);
    stepper_hal->timer_set_freq(motor->timer, 500);
    stepper_hal->timer_start_pulse(motor->timer);

    motor->current_dir = dir;
}



/* S型加减速部分 */
static motor_err_t scurve_generate_table(scurve_Handle_t *sc, uint32_t total_steps, uint8_t continuous,
                                  uint32_t accel_steps, uint32_t decel_steps);
motor_err_t speed_update(stepper_motor_t *motor);
static void speed_check(uint32_t speed) {
    if (speed > STEPPER_MAX_SPEED) speed = STEPPER_MAX_SPEED;
    if (speed < STEPPER_MIN_SPEED) speed = STEPPER_MIN_SPEED;
}

static void stepper_stop_speed(stepper_motor_t *motor) {
    if (!motor) return;

    motor->state = MOTOR_IDLE;
    motor->target_steps = 0;
    motor->current_steps = 0;
    if (motor->exit_continuous) {
        motor->exit_continuous = 0;
    }
    stepper_hal->timer_stop_pulse(motor->timer);

}

static void stepper_start_speed(stepper_motor_t *motor, uint32_t target_steps, uint32_t start_speed, Rotation_Dir dir) {
   motor->state = MOTOR_ACCEL;

   motor->current_steps = 0;
   motor->target_steps = target_steps;
   motor->current_dir = dir ? MOTOR_DIR_CW : MOTOR_DIR_CCW;

   stepper_hal->gpio_write(motor->dir_gpio, dir);
   stepper_hal->timer_set_freq(motor->timer, motor->s_curve->v_start);
   stepper_hal->timer_start_pulse(motor->timer);
}


void one_period_done_callback(stepper_motor_t *motor) {
    if (motor->state == MOTOR_IDLE) return;
    if (motor->current_steps < motor->target_steps) {
        speed_update(motor);
    }
    else {
        stepper_stop_speed(motor);
    }
}

motor_err_t Stepper_Start(MoveConfig_t * config, stepper_motor_t * motor) {
    if (!motor) return ERR_PARAM;
    if (!config) return ERR_PARAM;
    if (!(motor->state == MOTOR_IDLE)) return ERR_PARAM;

    speed_check(config->start_speed);
    speed_check(config->const_speed);
    speed_check(config->end_speed);

    motor->s_curve->v_start = config->start_speed;
    motor->s_curve->v_const = config->const_speed;
    motor->s_curve->v_end = config->end_speed;

    motor->current_dir = config->direction;

    motor->is_continuous = config->mode == MOVE_MODE_CONTINUOUS ?  1 : 0;


    if (scurve_generate_table(motor->s_curve, config->total_step, motor->is_continuous,
        config->acc_steps, config->dec_steps) == SET_OK)
    {
        stepper_start_speed(motor, config->total_step, config->start_speed, config->direction);
        return SET_OK;
    }
    else {
        return ERR_PARAM;
    }
}

motor_err_t stepper_continuous_stop(stepper_motor_t *motor) {
    if (!motor) return ERR_PARAM;
    motor->exit_continuous = 1;
    return SET_OK;
}


motor_err_t speed_update(stepper_motor_t *motor) {
    if (!motor) return ERR_PARAM;

    /* 空闲/停止状态直接退出 */
    if (motor->state == MOTOR_IDLE || motor->state == MOTOR_STOP) {
        return SET_OK;
    }

    scurve_Handle_t *sc = motor->s_curve;
    if (!sc) return ERR_PARAM;

    uint32_t step = motor->current_steps;
    uint32_t acc_end = sc->acc_step;
    uint32_t const_end = acc_end + sc->const_step;
    uint32_t total = acc_end + sc->const_step + sc->dec_step;

    if (motor->exit_continuous) {
        /* 收到停止信号，转入减速 */
        motor->is_continuous = 0;
        motor->exit_continuous = 0;      // 清除信号
        motor->state = MOTOR_DECEL;
    }

    /* ---------- 连续模式快速通道 ---------- */
    if (motor->is_continuous) {
        if (step < acc_end) {
            /* 仍在加速段：正常查表 */
            motor->state = MOTOR_ACCEL;
            if (step < SPEED_TABLE_SIZE) {
                stepper_hal->timer_set_freq(motor->timer, sc->speed_table[step]);
                motor->current_steps++;
            }
            return SET_OK;
        }

        /* 已进入匀速段：不写定时器，不累加步数，只检查退出信号 */
        motor->state = MOTOR_CONST;
        return SET_OK;   // 匀速时直接返回，无任何多余操作
    }

    /* ---------- 定位模式（包含连续模式退出后的减速） ---------- */
    if (step < acc_end) {
        motor->state = MOTOR_ACCEL;
    } else if (step < const_end) {
        motor->state = MOTOR_CONST;
    } else if (step < total) {
        motor->state = MOTOR_DECEL;
    } else {
        /* 运动完成 */
        stepper_stop_speed(motor);
        return SET_OK;
    }

    /* 输出速度表对应频率 */
    if (step < total && step < SPEED_TABLE_SIZE) {
        stepper_hal->timer_set_freq(motor->timer, sc->speed_table[step]);
        motor->current_steps++;
    }

    return SET_OK;
}

/**
 * @brief 生成七段S曲线速度表（支持定位模式/连续运转模式）
 * @param sc          速度表句柄
 * @param total_steps 总运动步数（仅定位模式有效，连续模式传0即可）
 * @param continuous  1=连续运转模式，0=定位模式
 * @param accel_steps 加速段步数（0则自动计算）
 * @param decel_steps 减速段步数（0则自动计算）
 * @return motor_err_t 错误码
 */
static motor_err_t scurve_generate_table(scurve_Handle_t *sc, uint32_t total_steps, uint8_t continuous,
                                  uint32_t accel_steps, uint32_t decel_steps) {
    // 1. 基础参数校验
    if (!sc) return ERR_PARAM;
    const uint32_t v0 = sc->v_start;   // 起始速度
    const uint32_t vm = sc->v_const;   // 匀速速度
    const uint32_t ve = sc->v_end;     // 停止速度
    if (v0 == 0 || vm == 0) return ERR_PARAM;

    // 确保目标速度不低于起始速度
    sc->v_const = (vm < v0) ? v0 : vm;

    // 2. 模式分离：初始化加减速/匀速步数
    uint32_t const_steps = 0;
    if (continuous) {
        // ====================== 连续运转模式 ======================
        const_steps = 0;
        // 自动计算加减速步数
        if (accel_steps == 0) accel_steps = 200;  // 默认加速步数
        if (decel_steps == 0) decel_steps = 200;  // 默认减速步数

        // 校验：加减速总步数不超过速度表容量
        if ((accel_steps + decel_steps) > SPEED_TABLE_SIZE) {
            return ERR_PARAM;
        }
    } else {
        // ====================== 定位模式 ======================
        // 总步数合法性校验
        if (total_steps == 0) return ERR_PARAM;
        if (accel_steps == 0 || decel_steps == 0) {
            // 默认比例：加速40% + 减速40% + 匀速20%
            accel_steps = total_steps * 40 / 100;
            decel_steps = total_steps * 40 / 100;
        }

        // 总步数不足时，等比例缩小加减速（无匀速段）
        if (total_steps < accel_steps + decel_steps) {
            accel_steps = total_steps * accel_steps / (accel_steps + decel_steps);
            decel_steps = total_steps - accel_steps;
        }
        const_steps = total_steps - accel_steps - decel_steps;
    }

    // 3. 保存分段步数（供speed_update函数使用）
    sc->acc_step   = accel_steps;
    sc->dec_step   = decel_steps;
    sc->const_step = const_steps;

    // 4. 清空速度表索引，开始生成
    uint32_t idx = 0;
    // 加速段：S曲线（两种模式共用）
    for (uint32_t i = 0; i < accel_steps && idx < SPEED_TABLE_SIZE; i++) {
        float t = (float)i / (float)accel_steps;
        float v = (float)v0 + (float)(sc->v_const - v0) * (3.0f * t * t - 2.0f * t * t * t);
        sc->speed_table[idx++] = (uint32_t)v;
    }
    // 匀速段（仅定位模式有效）
    for (uint32_t i = 0; i < const_steps && idx < SPEED_TABLE_SIZE; i++) {
        sc->speed_table[idx++] = sc->v_const;
    }
    // 减速段：两种模式都生成！（连续模式预留，退出时使用）
    for (uint32_t i = 0; i < decel_steps && idx < SPEED_TABLE_SIZE; i++) {
        float t = (float)i / (float)decel_steps;
        float v = (float)sc->v_const - (float)(sc->v_const - ve) * (3.0f * t * t - 2.0f * t * t * t);
        sc->speed_table[idx++] = (uint32_t)v;
    }

    sc->idx = 0;
    return SET_OK;
}




