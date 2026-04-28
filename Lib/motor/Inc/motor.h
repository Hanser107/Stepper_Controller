#ifndef INC_MOTOR_H
#define INC_MOTOR_H

#include "stm32f1xx_hal.h"
#include "can.h"
#include "bsp_can.h"
#include <string.h>



uint8_t motor_set_current(uint16_t motor_id, int16_t current);

#endif

