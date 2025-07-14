#ifndef STM32F446xx
#define STM32F446xx
#endif

#include "system_config.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"

static I2C_HandleTypeDef hi2c1 = {
    .Instance = I2C1,
    .Init = {
        .ClockSpeed = 10000, // 10 kHz standard mode
        .DutyCycle = I2C_DUTYCYCLE_2,
        .OwnAddress1 = 0,
        .AddressingMode = I2C_ADDRESSINGMODE_7BIT,
        .DualAddressMode = I2C_DUALADDRESS_DISABLE,
        .OwnAddress2 = 0,
        .GeneralCallMode = I2C_GENERALCALL_DISABLE,
        .NoStretchMode = I2C_NOSTRETCH_DISABLE
    }
};

int I2C1_Init(void)
{
    return HAL_I2C_Init(&hi2c1);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (i2cHandle->Instance == I2C1)
    {
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE(); // Assuming I2C1 on PB6 (SCL), PB7 (SDA)

        // Configure I2C1 SCL and SDA pins
        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD; // Open-drain for I2C
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
}

HAL_StatusTypeDef I2C_EEPROM_Write(uint16_t MemAddress, const uint8_t *data)
{
    uint8_t A98 = (MemAddress >> 8) & 0x03U;
    uint8_t addr = I2C_EEPROM_BASE_ADDR | A98 << 1;
    uint8_t buf[2] = {MemAddress & 0xFFU, data[0]};
    int res = HAL_I2C_Master_Transmit(&hi2c1, addr, buf, sizeof(buf), HAL_MAX_DELAY);
    // delay
    res = HAL_I2C_Master_Transmit(&hi2c1, addr, buf, 1, HAL_MAX_DELAY);
    return res;
}

HAL_StatusTypeDef I2C_EEPROM_Read(uint16_t MemAddress, uint8_t *pData)
{
    uint8_t A98 = (MemAddress >> 8) & 0x03U;
    uint8_t addr = I2C_EEPROM_BASE_ADDR | A98 << 1;
    uint8_t buf[1] = {MemAddress & 0xFFU};
    int res = HAL_I2C_Master_Transmit(&hi2c1, addr, buf, 1, HAL_MAX_DELAY);
    if (res != HAL_OK)
        return res;
    res = HAL_I2C_Master_Receive(&hi2c1, addr, pData, 1, HAL_MAX_DELAY);
    return res;
}
