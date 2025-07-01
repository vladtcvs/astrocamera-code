#include "system_config.h"
#include "usb_device.h"
#include "system.h"

#include "system_clock.h"
#include "i2c.h"
#include "qspi.h"
#include "spi.h"
#include "uart.h"
#include "gpio.h"
#include "usb.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

#define SENSORS_POLL_TASK_STACK_SIZE 200
static TaskHandle_t sensors_poll_task;
static StackType_t  sensors_poll_task_stack[SENSORS_POLL_TASK_STACK_SIZE];
static StaticTask_t sensors_poll_task_buffer;

static void sensors_poll_function(void *arg)
{
    while (1) {
        HAL_Delay(10);
    }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_QUADSPI_Init();
    MX_SPI4_Init();
    MX_UART5_Init();
    MX_USART1_UART_Init();
    void *usb_ctx = MX_USB_DEVICE_Init();
    if (usb_ctx == NULL)
        Error_Handler();

    sensors_poll_task = xTaskCreateStatic(sensors_poll_function,
                                          "sensors",
                                          SENSORS_POLL_TASK_STACK_SIZE,
                                          NULL,
                                          1,
                                          sensors_poll_task_stack,
                                          &sensors_poll_task_buffer);

    /* Infinite loop */
    vTaskStartScheduler();
    
    /* Should not reach here */
    while (1)
    {}
    return 0;
}

