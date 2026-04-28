#include "move.h"
#include "stepper.h"


uint8_t move_position(stepper_motor_t * motor) {
    MoveConfig_t config = {
        .mode = MOVE_MODE_CONTINUOUS,

        .direction = 1,

        .start_speed = 500,
        .const_speed = 2000,
        .end_speed = 500,

        .acc_steps = 1000,
        .dec_steps = 1000,
        .total_step = 2200,
      };
    Stepper_Start(&config, motor);
    return 1;
}
