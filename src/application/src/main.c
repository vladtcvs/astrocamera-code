#include "system_config.h"
#include "system.h"
#include "stm32f4xx.h"

#include "hw/system_clock.h"

#include "hw/i2c.h"
#include "hw/qspi.h"
#include "hw/spi.h"
#include "hw/uart.h"
#include "hw/gpio.h"
#include "hw/usb.h"
#include "usb_device.h"


#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <timers.h>
#include <semphr.h>

#define SENSORS_POLL_TASK_STACK_SIZE 200
static BaseType_t sensors_poll_task;

static void sensors_poll_function(void *ctx)
{
    int current_temperature = 2961; // 296.1 K = 23 C
    const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
    while (1) {
        //send_sensors((struct usb_context_s *)ctx, current_temperature);
        vTaskDelay(xDelay);
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

    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
    HAL_NVIC_SetPriority(SVCall_IRQn, 15, 0);

    /* Configure the system clock */
    SystemClock_Config();

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_QUADSPI_Init();
    MX_SPI4_Init();
    MX_UART5_Init();
    MX_USART1_UART_Init();
    struct usb_context_s* usb_ctx = MX_USB_DEVICE_Init();
    if (usb_ctx == NULL)
        Error_Handler();

    sensors_poll_task = xTaskCreate(sensors_poll_function,
                                          "sensors",
                                          SENSORS_POLL_TASK_STACK_SIZE,
                                          usb_ctx,
                                          1,
                                          NULL);

    /* Infinite loop */
    vTaskStartScheduler();
    
    /* Should not reach here */
    while (1)
    {}
    return 0;
}
