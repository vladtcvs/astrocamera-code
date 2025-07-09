#pragma once

#include <stdint.h>
#include "usbd_def.h"

typedef struct
{
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
} USBD_CAMERA_ItfTypeDef;

enum HID_InputReportId_e {
    CURRENT_TEMPERATURE = 0x01U,
    POWER_STATUS        = 0x02U,
    EXPOSURE_STATUS     = 0x03U,
};

enum HID_OutputReportId_e {
    TARGET_TEMPERATURE  = 0x01U,
    POWER_CTL           = 0x02U,
    EXPOSURE_CTL        = 0x03U,
    EXPOSURE_MODE_CTL   = 0x04U,
};

uint8_t USBD_CAMERA_Configure(unsigned fps, unsigned width, unsigned height, const char *FourCC);
uint8_t USBD_CAMERA_SendHIDReport(uint8_t epAddr, uint8_t *data, size_t len);

extern USBD_ClassTypeDef    USBD_CAMERA;
