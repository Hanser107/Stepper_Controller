#ifndef INC_BSP_INIT
#define INC_BSP_INIT

#include "stm32f1xx_hal.h"
#include "gpio_led.h"
#include "gpio_button.h"

#include "servo.h"
#include "stepper.h"
#include "motor.h"


extern gpio_led_t gpio_led1;
extern gpio_led_t gpio_led2;
extern gpio_button_t gpio_button;
extern button_event_t gpio_button_event;
extern Servo_Handle_t servo1;
extern stepper_motor_t* stepperMotor1;


uint8_t bsp_init(void);

#endif
