#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>
#include <stdbool.h>

/* 不透明指针，隐藏内部细节 */
typedef struct StepperMotor StepperMotor;


/* 电机配置（用户需要提供） */
typedef struct {
    /* 定时器配置 */
    void *htim;             /* TIM_HandleTypeDef* */
    uint32_t timer_channel; /* TIM_CHANNEL_1..4 */

    /* 方向引脚配置 */
    void *dir_port;         /* GPIO_TypeDef* */
    uint16_t dir_pin;
    bool dir_active_level;  /* 正向旋转时该引脚电平（true=高，false=低） */

    /* 使能引脚配置（可选，为NULL表示不使用） */
    void *en_port;
    uint16_t en_pin;
    bool en_active_level;   /* 使能有效时的电平 */

    /* 电机参数 */
    uint32_t steps_per_rev; /* 每转步数 */
} StepperConfig;

/* ---------- 速度配置结构体 ---------- */
typedef enum {
    MOVE_MODE_POSITION,     /* 定位运动：运行指定步数后停止（自动包含加减速） */
    MOVE_MODE_CONTINUOUS,   /* 持续运转：以目标速度一直运行，直到外部停止 */
    MOVE_MODE_DECEL_STOP    /* 减速停止：从当前速度按配置的加加速度减速至停止 */
} MoveMode;


typedef struct MoveConfig {
    MoveMode mode;

    uint8_t direction;          /* 方向：1=正向，0=反向 */

    /* 速度曲线参数 */
    uint32_t start_speed;        /* 起始频率（Hz） */
    uint32_t const_speed;       /* 目标匀速频率（Hz） */
    uint32_t end_speed;         /* 停止频率（Hz） */
    uint32_t acceleration;      /* 加速度（Hz/s） */
    uint32_t jerk;              /* 加加速度（Hz/s²） */

    uint32_t total_step;        /* 总运动步数（持续运转时填0） */
}MoveConfig_t;

/* 电机状态枚举 */
typedef enum {
    MOTOR_IDLE,
    MOTOR_ACCEL,
    MOTOR_CONST,
    MOTOR_DECEL,
    MOTOR_STOP,
} MotorState;


/* 创建电机实例 */
StepperMotor* stepper_create(const StepperConfig *cfg);

/* 销毁电机实例 */
void stepper_destroy(StepperMotor *motor);

/* 使能/失能电机 */
void stepper_enable(StepperMotor *motor, bool enable);

/* 停止运动（急停，清空剩余步数） */
void stepper_stop(StepperMotor *motor);

/* 检查电机是否空闲（无运动） */
MotorState stepper_is_idle(StepperMotor *motor);

void stepper_test_run(StepperMotor *motor, uint32_t dir);

/* 电机单周期回调函数 */
void one_period_done_callback(StepperMotor *motor);

#endif