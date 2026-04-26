#include "servo.h"

#define SERVO_MIN_PULSE 500
#define SERVO_MAX_PULSE 2500
#define SERVO_RANGE (SERVO_MAX_PULSE - SERVO_MIN_PULSE)

void Servo_Init(Servo_Handle_t *servo, TIM_HandleTypeDef *htim,
                uint32_t channel, uint32_t timer_freq, uint16_t prescaler)  {
    if (servo == NULL) return;
    servo->htim = htim;
    servo->Channel = channel;
    servo->timer_freq = timer_freq;
    servo->timer_prescaler = prescaler;

    uint32_t arr = 0;
    arr = servo->timer_freq / servo->timer_prescaler / 50;
    if (arr > 65536) arr = 65536;

    __HAL_TIM_SET_AUTORELOAD(servo->htim, 20000 - 1);
    __HAL_TIM_SET_COMPARE(servo->htim, servo->Channel, 500);

    HAL_TIM_Base_Start(servo->htim);
    HAL_TIM_PWM_Start(servo->htim, servo->Channel);
}

static uint32_t AngleToCompare(Servo_Handle_t * servo, uint16_t angle) {
    //uint32_t arr = __HAL_TIM_GET_AUTORELOAD(servo->htim) + 1;
    return SERVO_MIN_PULSE + (uint32_t)(angle * SERVO_RANGE / 180);
}

void Servo_SetAngle(Servo_Handle_t * servo, uint16_t angle) {
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;

    uint32_t compare = AngleToCompare(servo, angle);
    __HAL_TIM_SET_COMPARE(servo->htim, servo->Channel, compare);

    servo->Angle = angle;
}

void Servo_SetRun(Servo_Handle_t * servo, uint32_t compare) {
    if (servo == NULL) return;
    if (compare < 500) compare = 500;

    HAL_TIM_Base_Stop(servo->htim);
    HAL_TIM_PWM_Stop(servo->htim, servo->Channel);

    __HAL_TIM_SET_COMPARE(servo->htim, servo->Channel, compare);

    HAL_TIM_Base_Start(servo->htim);
    HAL_TIM_PWM_Start(servo->htim, servo->Channel);
}


