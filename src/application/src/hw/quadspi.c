#ifndef STM32F446xx
#define STM32F446xx
#endif

#include "system_config.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "hw/quadspi.h"

#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>
#include <timers.h>

#define HARD_QPI 1

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
    cmd.Instruction       = 0xEB;  // Fast Read (with dummy cycles)
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize       = QSPI_ADDRESS_32_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;  // Single data output
    cmd.DummyCycles       = 8;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    memMappedCfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;

    return HAL_QSPI_MemoryMapped(&hqspi, &cmd, &memMappedCfg);
}

int QUADSPI_Init(void)
{
#if HARD_QPI
    hqspi.Instance = QUADSPI;
    hqspi.Init.ClockPrescaler     = 255;          // fQSPI = fAHB / (1 + ClockPrescaler)
    hqspi.Init.FifoThreshold      = 4;
    hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize          = POSITION_VAL(SRAM_SIZE + FPGA_FLASH_SIZE) - 1; // 16 MB + 512 KB
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_0;
    hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;
    return HAL_QSPI_Init(&hqspi);
#else
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // Configure NSS (PB6)
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    // Configure CLK (PD3)
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);

    // Configure MOSI (PC9)
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

    // Configure MISO (PC10)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    return 0;
#endif
}

static uint8_t transferByte(uint8_t out)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
    vTaskDelay(1);
    int i;
    uint8_t rcv = 0;
    for (i = 0; i < 8; i++) {
        int bit = (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_10) == GPIO_PIN_SET);
        rcv = (rcv << 1) | bit;
        bit = (out & 0x80U);
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, bit ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
        out <<= 1;
        //vTaskDelay(1);
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
        //vTaskDelay(1);
    }
    return rcv;
}

HAL_StatusTypeDef QUADSPI_Read(uint32_t address, uint8_t *buffer, uint32_t size)
{
#if HARD_QPI
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = 0x03;                   // READ command
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 8;
    sCommand.NbData            = size;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    // Send the read command
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return HAL_ERROR;

    // Receive the data
    if (HAL_QSPI_Receive(&hqspi, buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return HAL_ERROR;
#else

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // CS = 0
    transferByte(0x03U);
    transferByte((address >> 16) & 0xFFU);
    transferByte((address >> 8) & 0xFFU);
    transferByte((address) & 0xFFU);
    transferByte(0);
    unsigned i;
    for (i = 0; i < size; i++)
        buffer[i] = transferByte(0);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // CS = 1
#endif
    return HAL_OK;
}

HAL_StatusTypeDef QUADSPI_Write(uint32_t address, uint8_t *buffer, uint32_t size)
{
#if HARD_QPI
    QSPI_CommandTypeDef sCommand = {0};

    sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    sCommand.Instruction       = 0x02;                   // WRITE command
    sCommand.AddressMode       = QSPI_ADDRESS_1_LINE;
    sCommand.AddressSize       = QSPI_ADDRESS_24_BITS;
    sCommand.Address           = address;
    sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode          = QSPI_DATA_1_LINE;
    sCommand.DummyCycles       = 0;
    sCommand.NbData            = size;
    sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    // Send the read command
    if (HAL_QSPI_Command(&hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return HAL_ERROR;

    // Transmit the data
    if (HAL_QSPI_Transmit(&hqspi, buffer, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return HAL_ERROR;
#else

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET); // CS = 0
    
    transferByte(0x02U);
    transferByte((address >> 16) & 0xFFU);
    transferByte((address >> 8) & 0xFFU);
    transferByte((address) & 0xFFU);
    unsigned i;
    for (i = 0; i < size; i++)
        transferByte(buffer[i]);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET); // CS = 1
#endif

    return HAL_OK;

}
