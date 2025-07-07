#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Frequency MHz */
#define HSE_VALUE 8000000UL                      // External oscillator
#define FREQ_MHZ 96U
#define FLASH_LATENCY FLASH_LATENCY_3            // See table from "3.4Read interface"
#define POWER_VOS (PWR_CR_VOS_0 | PWR_CR_VOS_0); // See notes from "3.4 Read interface"

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

/* USB */
#define DEVICE_USB_VID 1155
#define DEVICE_USB_PID 22315
#define DEVICE_VERSION 0x0001U

#ifdef __cplusplus
}
#endif
