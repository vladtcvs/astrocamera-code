#include <FreeRTOS.h>
#include <task.h>

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    while (1)
    {}
}

void vApplicationTickHook(void)
{
}

void vApplicationIdleHook(void)
{
}
