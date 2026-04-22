#ifndef STEPPER_H
#define STEPPER_H

#include <stdint.h>


#define SPEED_TABLE_SIZE 3200

#define STEPPER_MIN_SPEED 100
#define STEPPER_MAX_SPEED 3000

/* 不透明指针，隐藏内部细节 */
typedef struct stepper_motor_t stepper_motor_t;


/* 电机配置（用户需要提供） */
typedef struct {
    /* 定时器配置 */
    void *htim;             /* TIM_HandleTypeDef* */
    uint32_t timer_channel; /* TIM_CHANNEL_1..4 */

    /* 方向引脚配置 */
    void *dir_port;         /* GPIO_TypeDef* */
    uint16_t dir_pin;
    uint8_t dir_active_level;  /* 正向旋转时该引脚电平（true=高，false=低） */

    /* 使能引脚配置（可选，为NULL表示不使用） */
    void *en_port;
    uint16_t en_pin;
    uint8_t en_active_level;   /* 使能有效时的电平 */

    /* 电机参数 */
    uint32_t steps_per_rev; /* 每转步数 */
} StepperConfig;


typedef enum{
    MOTOR_BUSY,
    ERR_PARAM,
    SET_OK
}motor_err_t;

typedef enum {
    MOVE_MODE_POSITION,     /* 定位运动：运行指定步数后停止（自动包含加减速） */
    MOVE_MODE_CONTINUOUS,   /* 持续运转：以目标速度一直运行，直到外部停止 */
} MoveMode;

typedef enum {
    MOTOR_DIR_CW,
    MOTOR_DIR_CCW,
} Rotation_Dir;

/* ---------- 速度配置结构体 ---------- */
typedef struct MoveConfig {
    MoveMode mode;

    Rotation_Dir direction;          /* 方向：1=正向，0=反向 */

    /* 速度曲线参数 */
    uint32_t start_speed;        /* 起始频率（Hz） */
    uint32_t const_speed;       /* 目标匀速频率（Hz） */
    uint32_t end_speed;         /* 停止频率（Hz） */

    uint32_t acc_steps;         /* 加速步数 */
    uint32_t dec_steps;         /* 减速步数 */
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
stepper_motor_t* stepper_create(const StepperConfig *cfg);

/* 销毁电机实例 */
void stepper_destroy(stepper_motor_t *motor);

/* 使能/失能电机 */
void stepper_enable(stepper_motor_t *motor, uint8_t enable);

/* 停止运动（急停，清空剩余步数） */
void stepper_stop(stepper_motor_t *motor);

/* 检查电机是否空闲（无运动） */
MotorState stepper_is_idle(stepper_motor_t *motor);

void stepper_test_run(stepper_motor_t *motor, uint32_t dir);

/* 电机单周期回调函数 */
void one_period_done_callback(stepper_motor_t *motor);

motor_err_t Stepper_Start(MoveConfig_t * config, stepper_motor_t * motor);

#endif