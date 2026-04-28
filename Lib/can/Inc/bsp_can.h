#ifndef INC_BSP_CAN_H_
#define INC_BSP_CAN_H_

#include "stm32f1xx_hal.h"
#include "main.h"

/* CAN消息结构体 */
typedef struct {
    uint32_t id;      /* CAN ID */
    uint8_t dlc;      /* 数据长度 */
    uint8_t *data;  /* 数据内容 */
} CanMessage_t;

void BSP_CAN_FilterInit(CAN_HandleTypeDef *CanHandle);
void BSP_CAN_Start(CAN_HandleTypeDef *CanHandle);
HAL_StatusTypeDef CAN_Send(CAN_HandleTypeDef *CanHandle, CanMessage_t* msg, uint8_t *data);

#endif
