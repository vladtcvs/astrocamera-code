/*
 * Copyright (c) 2025 Vladislav Tsendrovskii
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "system_config.h"
#include "stm32f4xx.h"
#include "stm32f446xx.h"

#include "hw/pll.h"
#include "hw/usb.h"
#include "hw/i2c.h"
#include "hw/uart.h"
#include "hw/spi.h"
#include "hw/quadspi.h"
#include "hw/fpga-ctl.h"
#include "usb_device.h"

#include "core.h"
#include "config.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

#define SENSORS_POLL_TASK_STACK_SIZE 200
static StackType_t  sensors_poll_task_stack[SENSORS_POLL_TASK_STACK_SIZE];
static TaskHandle_t sensors_poll_task;
static StaticTask_t sensors_poll_task_buffer;

#define SERIAL_SEND_TASK_STACK_SIZE 200
static StackType_t  serial_send_task_stack[SERIAL_SEND_TASK_STACK_SIZE];
static TaskHandle_t serial_send_task;
static StaticTask_t serial_send_task_buffer;

struct config_s config;

extern bool freertos_tick;

__attribute__((section(".noinit"))) uint32_t dfu_flag;

void reboot_in_dfu(void)
{
    dfu_flag = 0xDEADBEEFU;
    NVIC_SystemReset();
}

void reboot_in_app(void)
{
    dfu_flag = 0;
    NVIC_SystemReset();
}

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    // Set system IRQ priorities
    HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
    HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
    HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);

    // Important for FreeRTOS
    HAL_NVIC_SetPriority(SVCall_IRQn, 15, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

static bool is_dfu(void)
{
    /*
    uint32_t reset_flags = RCC->CSR;
    if (reset_flags & (RCC_CSR_PORRSTF)) {
        // Hard reset: init dfu_flag
        dfu_flag = 0;
        return false;
    }

    RCC->CSR |= RCC_CSR_RMVF;  // Always clear flags after checking*/
    return (dfu_flag == 0xDEADBEEFU);
}

int main(void)
{
    // Disable sending SysTick to FreeRTOS kernel
    freertos_tick = false;
    HAL_Init();

    PLL_Config();
    SYSCLK_Config();
    I2C1_Init();
    QUADSPI_Init();
    QSPI_EnableMemoryMapped();
    FPGA_CTL_Init();

    bool boot_dfu = is_dfu();

    if (boot_dfu)
    {
        struct usb_context_s *usb_ctx = USB_DEVICE_Init_DFU();
        if (usb_ctx == NULL)
            goto error;

        // Enable sending SysTick to FreeRTOS kernel
        freertos_tick = true;
    }
    else
    {
        USART1_Init(115200);
        UART5_Init(9600);
        SPI4_Init();

        load_config(&camera_config);
        struct usb_context_s *usb_ctx = USB_DEVICE_Init(2, camera_config.width, camera_config.height, camera_config.FourCC);
        if (usb_ctx == NULL)
            goto error;

        // Enable sending SysTick to FreeRTOS kernel
        freertos_tick = true;

        core_init(usb_ctx);
/*        sensors_poll_task = xTaskCreateStatic(core_sensors_poll_function,
                                              "sensors",
                                              SENSORS_POLL_TASK_STACK_SIZE,
                                              NULL,
                                              1,
                                              sensors_poll_task_stack,
                                              &sensors_poll_task_buffer);
*/
        serial_send_task = xTaskCreateStatic(core_serial_send_function,
                                              "serial_send",
                                              SERIAL_SEND_TASK_STACK_SIZE,
                                              NULL,
                                              1,
                                              serial_send_task_stack,
                                              &serial_send_task_buffer);
    }
    vTaskStartScheduler();

error:
    while (1)
        ;
    return 0;
}
