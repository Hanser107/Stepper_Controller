#ifndef INC_SERVO_H
#define INC_SERVO_H

#include "stm32f1xx_hal.h"

typedef struct {
    TIM_HandleTypeDef* htim;
    uint32_t Channel;
    uint32_t timer_freq;
    uint16_t timer_prescaler;

    uint16_t Angle;
}Servo_Handle_t;

void Servo_Init(Servo_Handle_t *servo, TIM_HandleTypeDef *htim,
                uint32_t channel, uint32_t timer_freq, uint16_t prescaler);
void Servo_SetAngle(Servo_Handle_t * servo, uint16_t angle);
void Servo_SetRun(Servo_Handle_t * servo, uint32_t compare);

#endif
