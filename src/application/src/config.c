#include <config.h>
#include <hw/i2c.h>

struct config_s camera_config;

void load_config(struct config_s *cfg)
{
    I2C_EEPROM_Read(0, &cfg->FourCC[0]);
    I2C_EEPROM_Read(1, &cfg->FourCC[1]);
    I2C_EEPROM_Read(2, &cfg->FourCC[2]);
    I2C_EEPROM_Read(3, &cfg->FourCC[3]);

    uint8_t w_H;
    uint8_t w_L;
    uint8_t h_H;
    uint8_t h_L;

    I2C_EEPROM_Read(4, &w_H);
    I2C_EEPROM_Read(5, &w_L);
    cfg->width = ((uint16_t)w_H)<<8 | w_L;

    I2C_EEPROM_Read(6, &h_H);
    I2C_EEPROM_Read(7, &h_L);
    cfg->height = ((uint16_t)h_H)<<8 | h_L;
}
