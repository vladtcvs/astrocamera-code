#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"

struct usb_context_s;

struct usb_context_s* USB_DEVICE_Init(unsigned fps, unsigned width, unsigned height, const char *FourCC);
struct usb_context_s* USB_DEVICE_Init_DFU(void);

uint8_t send_current_temperature(int16_t current_temperature);
uint8_t send_power_settings(bool TEC, bool fan, int window_heater);
uint8_t send_shutter(bool exposure);
uint8_t send_serial_data(const uint8_t *data, size_t len);

struct usb_context_s {
    uint8_t (*set_target_temperature)(unsigned temperature);
    uint8_t (*set_power_settings)(bool TEC, bool fan, int window_heater);
    uint8_t (*exposure)(unsigned exposure);
    uint8_t (*exposure_mode)(unsigned exposure_mode);
    uint8_t (*serial_data)(const uint8_t *data, size_t len);
    uint8_t current_temperature_buf[3];
    uint8_t power_status_buf[2];
    uint8_t exposure_status_buf[2];
};

#ifdef __cplusplus
}
#endif
