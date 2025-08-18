#ifndef STM32F446xx
#define STM32F446xx
#endif

#include <stm32f4xx_hal.h>
#include <stm32f446xx.h>
#include "system_config.h"

#include "hw/fpga-ctl.h"

int FPGA_CTL_Init(void)
{
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = INITN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(INITN_GPIO_Port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(PROGRAMN_GPIO_Port, PROGRAMN_Pin, GPIO_PIN_SET);
    GPIO_InitStruct.Pin = PROGRAMN_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PROGRAMN_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = DONE_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DONE_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COMMAND_INT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(COMMAND_INT_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COMMAND_TRG_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(COMMAND_TRG_GPIO_Port, &GPIO_InitStruct);

    return HAL_OK;
}
