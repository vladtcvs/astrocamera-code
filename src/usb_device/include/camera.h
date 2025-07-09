#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "usbd_def.h"

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

struct USBD_CAMERA_callbacks_t {
    uint8_t (*HID_OutputReport)(uint8_t report_id, const uint8_t *data, size_t len, bool from_interrupt);
    uint8_t *(*HID_GetInReport)(uint8_t report_id, size_t* len);
};


uint8_t USBD_CAMERA_Configure(unsigned fps, unsigned width, unsigned height, const char *FourCC);
uint8_t USBD_CAMERA_HID_SendReport(USBD_HandleTypeDef *pdev, const uint8_t *data, size_t len);
uint8_t USBD_CAMERA_RegisterInterface(USBD_HandleTypeDef *pdev, struct USBD_CAMERA_callbacks_t* cbs);

extern USBD_ClassTypeDef    USBD_CAMERA;
