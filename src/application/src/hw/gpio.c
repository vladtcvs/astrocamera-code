#include "stm32f4xx.h"

#include "hw/gpio.h"
#include "system.h"
#include "system_config.h"

int MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(MODBUS_DIR_GPIO_Port, MODBUS_DIR_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOD, LED2_Pin | LED1_Pin | PROGRAMN_Pin | COMMAND_TRG_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : MODBUS_DIR_Pin */
    GPIO_InitStruct.Pin = MODBUS_DIR_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MODBUS_DIR_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pins : LED2_Pin LED1_Pin */
    GPIO_InitStruct.Pin = LED2_Pin | LED1_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*Configure GPIO pins : INITN_Pin DONE_Pin COMMAND_INT_Pin */
    GPIO_InitStruct.Pin = INITN_Pin | DONE_Pin | COMMAND_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*Configure GPIO pins : PROGRAMN_Pin COMMAND_TRG_Pin */
    GPIO_InitStruct.Pin = PROGRAMN_Pin | COMMAND_TRG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    return HAL_OK;
}
