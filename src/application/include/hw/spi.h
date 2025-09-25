#pragma once

#include <stdint.h>

int SPI4_Init(void);
void SPI4_SetCS(int level);
uint8_t SPI4_Transfer(uint8_t data);
