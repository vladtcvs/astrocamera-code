
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 Vladislav Tsendrovskii
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

    /* Private includes ----------------------------------------------------------*/

    /* Exported types ------------------------------------------------------------*/

    /* Exported constants --------------------------------------------------------*/

    /* Exported macro ------------------------------------------------------------*/

    /* Exported functions prototypes ---------------------------------------------*/
    void Error_Handler(void);

/* Private defines -----------------------------------------------------------*/
#define USB_RST_Pin GPIO_PIN_1
#define USB_RST_GPIO_Port GPIOC
#define MODBUS_DIR_Pin GPIO_PIN_9
#define MODBUS_DIR_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOD
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOD
#define INITN_Pin GPIO_PIN_13
#define INITN_GPIO_Port GPIOD
#define PROGRAMN_Pin GPIO_PIN_14
#define PROGRAMN_GPIO_Port GPIOD
#define DONE_Pin GPIO_PIN_15
#define DONE_GPIO_Port GPIOD
#define COMMAND_INT_Pin GPIO_PIN_4
#define COMMAND_INT_GPIO_Port GPIOD
#define COMMAND_TRG_Pin GPIO_PIN_5
#define COMMAND_TRG_GPIO_Port GPIOD

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
