#include "motor.h"

#include "can.h"


struct motor_struct_t
{
    uint8_t motor_id;
    uint8_t motor_state;
    uint8_t motor_dir;

    uint32_t motor_current;
    uint32_t motor_angle;

};

void motor_init(void) {

}


uint8_t motor_set_current(uint16_t motor_id, int16_t current) {
    CanMessage_t msg;
    uint8_t data[8];
    msg.id = motor_id;
    msg.dlc = 8;
    memset(data,0,8);
    data[2] = current >> 8;
    data[3] = current;

    if (CAN_Send(&hcan,&msg, data) == HAL_OK)
    return 1;
    else return 0;
}