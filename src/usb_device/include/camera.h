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
    uint8_t (*VS_StartStream)(void);
    uint8_t (*VS_StopStream)(void);

    uint8_t (*VC_SetGain)(unsigned gain);
    uint8_t (*VC_GetGain)(unsigned *gain);
    
    uint8_t (*VC_GetFan)(unsigned *fan);
    uint8_t (*VC_GetTec)(unsigned *tec);
    uint8_t (*VC_GetWindowHeater)(unsigned *heater);
    uint8_t (*VC_GetTriggerMode)(unsigned *trigger_mode);
    uint8_t (*VC_GetTargetTemperature)(unsigned *temperature);
    uint8_t (*VC_GetCurrentTemperature)(unsigned *temperature);
    uint8_t (*VC_GetWindowTemperature)(unsigned *temperature);

    uint8_t (*VC_SetFan)(unsigned fan);
    uint8_t (*VC_SetTec)(unsigned tec);
    uint8_t (*VC_SetWindowHeater)(unsigned heater);
    uint8_t (*VC_SetTriggerMode)(unsigned trigger_mode);
    uint8_t (*VC_SetTargetTemperature)(unsigned temperature);

    uint8_t (*VC_SetExposure)(uint32_t exposure);
    uint8_t (*VC_GetExposure)(uint32_t *exposure);
    uint8_t (*CDC_ACM_Control)(uint8_t request, uint8_t *data, size_t len);
    uint8_t (*CDC_DATA_DataOut)(const uint8_t *data, size_t len);
};


uint8_t USBD_CAMERA_Configure(unsigned fps, unsigned width, unsigned height, const char *FourCC);
uint8_t USBD_CAMERA_Configure_DFU(void);

uint8_t USBD_CAMERA_CDC_DATA_SendSerial(USBD_HandleTypeDef *pdev, const uint8_t *data, size_t len);

uint8_t USBD_CAMERA_RegisterInterface(USBD_HandleTypeDef *pdev, struct USBD_CAMERA_callbacks_t* cbs);

extern USBD_ClassTypeDef    USBD_CAMERA;
