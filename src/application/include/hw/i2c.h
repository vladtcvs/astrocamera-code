#pragma once
#include <stdint.h>

int I2C1_Init(void);

HAL_StatusTypeDef I2C_EEPROM_Write(uint16_t MemAddress, const uint8_t *pData);

HAL_StatusTypeDef I2C_EEPROM_Read(uint16_t MemAddress, uint8_t *pData);
