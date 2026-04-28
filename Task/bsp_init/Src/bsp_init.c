#include "bsp_init.h"
#include "main.h"
#include "can.h"
#include "tim.h"



gpio_led_t gpio_led1;
gpio_led_t gpio_led2;
gpio_button_t gpio_button;
button_event_t gpio_button_event;
Servo_Handle_t servo1;
stepper_motor_t* stepperMotor1;

uint8_t bsp_init(void) {
    /* 初始化基本板载外设 */
    gpio_led_init(&gpio_led1, LED1_GPIO_Port, LED1_Pin);
    gpio_led_init(&gpio_led2, LED2_GPIO_Port, LED2_Pin);
    gpio_button_init(&gpio_button, KEY1_GPIO_Port, KEY1_Pin, 0);

    BSP_CAN_FilterInit(&hcan);
    BSP_CAN_Start(&hcan);

    /* 初始化电机，舵机外设 */
    StepperConfig StepperConfigStruct = {
        .htim = &htim2,
        .timer_channel = TIM_CHANNEL_1,
        .dir_port = Motor1_Dir_GPIO_Port,
        .dir_pin = Motor1_Dir_Pin,
        .dir_active_level = 1,

        .en_port = Motor1_Ena_GPIO_Port,
        .en_pin = Motor1_Ena_Pin,
        .en_active_level = 0,

        .steps_per_rev = 3200,
      };
    stepperMotor1 = stepper_create(&StepperConfigStruct);
    if(!stepperMotor1) {
        return -1;
    }
    //Servo_Init(&servo1, &htim2, TIM_CHANNEL_1, 72000000, 72);
    return 1;
}



