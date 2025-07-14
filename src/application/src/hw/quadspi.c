#ifndef STM32F446xx
#define STM32F446xx
#endif

#include "system_config.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"

QSPI_HandleTypeDef hqspi;

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (qspiHandle->Instance == QUADSPI)
    {
        /* Enable peripheral clocks */
        __HAL_RCC_QSPI_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_GPIOE_CLK_ENABLE();

        /* QUADSPI CLK -> PD3 */
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_QSPI;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* QUADSPI NCS -> PB6 */
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF10_QSPI;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* QUADSPI IO0, IO1 -> PC9, PC10 */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_QSPI;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        /* QUADSPI IO2 -> PE2 */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_QSPI;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

        /* QUADSPI IO3 -> PA1 */
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_QSPI;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

int QSPI_EnableMemoryMapped(void)
{
    QSPI_CommandTypeDef cmd = {0};
    QSPI_MemoryMappedTypeDef memMappedCfg = {0};

    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = 0x0B;  // Fast Read (with dummy cycles)
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize       = QSPI_ADDRESS_32_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_4_LINES;  // Quad data output
    cmd.DummyCycles       = 8;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    memMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    return HAL_QSPI_MemoryMapped(&hqspi, &cmd, &memMappedCfg);
}

int QUADSPI_Init(void)
{
    hqspi.Instance = QUADSPI;
    hqspi.Init.ClockPrescaler     = 1;          // fQSPI = fAHB / (1 + ClockPrescaler)
    hqspi.Init.FifoThreshold      = 4;
    hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize          = POSITION_VAL(SRAM_SIZE + FPGA_FLASH_SIZE) - 1; // 16 MB + 512 KB
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;
    return HAL_QSPI_Init(&hqspi);
}
