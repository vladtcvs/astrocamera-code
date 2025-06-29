#include "system.h"
#include "stm32f4xx.h"

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
