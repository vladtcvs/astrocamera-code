#define STM32F446xx

#include "hw/pll.h"

#include "system_config.h"
#include "stm32f4xx.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"

int PLL_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {
        .OscillatorType = RCC_OSCILLATORTYPE_HSE,
        .HSEState = RCC_HSE_ON,
        .PLL = {
            .PLLState = RCC_PLL_ON,
            .PLLSource = RCC_PLLSOURCE_HSE,
            .PLLM = 8U,
            .PLLN = FREQ_MHZ*2,
            .PLLP = RCC_PLLP_DIV2,
            .PLLQ = FREQ_MHZ*2 / 48U,   // To get 48 MHz
            .PLLR = 2,                  // For I2S, SAI, SYSTEM, SPDIFRX. SYSTEM uses PLLP, and other I don't use
        },
    };
    int res;
    if ((res = HAL_RCC_OscConfig(&RCC_OscInitStruct)) != HAL_OK)
        return res;
    return HAL_OK;
}

int SYSCLK_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {
        .ClockType = RCC_CLOCKTYPE_SYSCLK |     // System Clock
                     RCC_CLOCKTYPE_HCLK   |     // AHB clock
                     RCC_CLOCKTYPE_PCLK1  |     // APB1
                     RCC_CLOCKTYPE_PCLK2,       // APB2
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,       // FREQ_MHZ
        .APB1CLKDivider = RCC_HCLK_DIV4,        // FREQ_MHZ/4
        .APB2CLKDivider = RCC_HCLK_DIV2,        // FREQ_MHZ/2
    };

    PWR->CR &= ~(PWR_CR_VOS);   // Reset VOS bits
    PWR->CR |= POWER_VOS;       // Select VOS

    volatile int i;
    for (i = 0; i < 10000; i++) // wait
        asm("nop");
    int res = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY);

    if (res != HAL_OK)
        return res;
    for (i = 0; i < 10000; i++) // wait
        asm("nop");
    return HAL_OK;
}
