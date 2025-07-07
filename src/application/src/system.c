#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while (1)
    {
    }
}

void vApplicationTickHook(void)
{
}

void vApplicationIdleHook(void)
{
}


void NMI_Handler(void)
{
    while (1)
        ;
}

void MemManage_Handler(void)
{
    while (1)
        ;
}

void BusFault_Handler(void)
{
    while (1)
        ;
}

void UsageFault_Handler(void)
{
    while (1)
        ;
}

/* Defined by FreeRTOS
void SVC_Handler(void)
{
}
*/

void DebugMon_Handler(void)
{
}


/* Defined by FreeRTOS
void PendSV_Handler(void)
{
}
*/

void xPortSysTickHandler(void);
bool freertos_tick = false;
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (freertos_tick)
        xPortSysTickHandler();
}
