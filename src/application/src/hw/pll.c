#define STM32F446xx

#include "hw/pll.h"
#include "system_config.h"

#include "stm32f4xx.h"
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
            .PLLN = FREQ_MHZ * 2U,
            .PLLP = RCC_PLLP_DIV2,
            .PLLQ = (FREQ_MHZ * 2U) / 48U, // USB/SDIO/RNG @ 48 MHz
            .PLLR = 2U                     // Not used, safe to leave at 2
        }};

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}

int SYSCLK_Config(void)
{
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {
        .ClockType = RCC_CLOCKTYPE_SYSCLK |
                     RCC_CLOCKTYPE_HCLK |
                     RCC_CLOCKTYPE_PCLK1 |
                     RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV4,
        .APB2CLKDivider = RCC_HCLK_DIV2};

    // Set voltage scaling (VOS)
    PWR->CR &= ~PWR_CR_VOS;
    PWR->CR |= POWER_VOS;

    // Wait for voltage regulator to stabilize
    for (volatile int i = 0; i < 10000; i++)
    {
        __NOP();
    }

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY) != HAL_OK)
        return HAL_ERROR;

    // Wait again after clock switch
    for (volatile int i = 0; i < 10000; i++)
    {
        __NOP();
    }

    return HAL_OK;
}
