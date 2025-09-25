#include <stdint.h>
#include "system_config.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal.h"
#include "hw/spi.h"

void ctl_spi_begin(void)
{
    SPI4_SetCS(0);
}

void ctl_spi_finish(void)
{
    SPI4_SetCS(1);
}

uint8_t ctl_spi_transfer(uint8_t data)
{
    return SPI4_Transfer(data);
}
