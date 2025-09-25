#pragma once

#include "stm32f4xx_hal.h"
#include <stdint.h>

int QUADSPI_Init(void);
int QSPI_EnableMemoryMapped(void);

HAL_StatusTypeDef QUADSPI_Read(uint32_t address, uint8_t *buffer, uint32_t size);
HAL_StatusTypeDef QUADSPI_Write(uint32_t address, uint8_t *buffer, uint32_t size);
