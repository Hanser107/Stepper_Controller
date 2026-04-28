#include "bsp_can.h"

void BSP_CAN_FilterInit(CAN_HandleTypeDef *CanHandle) {

    CAN_FilterTypeDef sFilterConfig;
    /* 配置CAN过滤器 */
    sFilterConfig.FilterBank = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    sFilterConfig.SlaveStartFilterBank = 14;

    if (HAL_CAN_ConfigFilter(CanHandle, &sFilterConfig) != HAL_OK)
    {
        /* 错误处理 */
        Error_Handler();
    }

}

void BSP_CAN_Start(CAN_HandleTypeDef *CanHandle) {
    if (HAL_CAN_Start(CanHandle) != HAL_OK)
    {
        /* 错误处理 */
        Error_Handler();
    }

    /* 使能CAN接收中断 */
    if (HAL_CAN_ActivateNotification(CanHandle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        /* 错误处理 */
        Error_Handler();
    }
}


/**
  * @brief  CAN发送函数
  * @param  msg: 要发送的CAN消息
  * @retval HAL状态
  */
HAL_StatusTypeDef CAN_Send(CAN_HandleTypeDef *CanHandle, CanMessage_t* msg, uint8_t *data)
{
    CAN_TxHeaderTypeDef txHeader;
    uint32_t txMailbox;

    /* 配置发送头 */
    txHeader.StdId = msg->id;
    txHeader.ExtId = 0;
    txHeader.RTR = CAN_RTR_DATA;
    txHeader.IDE = CAN_ID_STD;
    txHeader.DLC = msg->dlc;
    txHeader.TransmitGlobalTime = DISABLE;

    /* 发送消息 */
    return HAL_CAN_AddTxMessage(CanHandle, &txHeader, data, &txMailbox);
}

/**
  * @brief  CAN接收中断处理
  * @param  None
  * @retval None
  */

