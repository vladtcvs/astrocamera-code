#pragma once

#include <stdint.h>

struct config_s {
    uint16_t width;
    uint16_t height;
    char FourCC[4];
};

extern struct config_s camera_config;

void load_config(struct config_s *cfg);
