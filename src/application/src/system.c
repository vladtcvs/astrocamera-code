#include "system_config.h"
#include "system.h"
#include "stm32f4xx.h"

#include <FreeRTOS.h>
#include <task.h>

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{

    __disable_irq();
    while (1)
    {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{

ex:
    printf("Wrong parameters value: file %s on line %d\r\n", file, line) * /
}
#endif /* USE_FULL_ASSERT */

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
