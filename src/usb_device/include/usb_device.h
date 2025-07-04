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

/** USB Device initialization function. */
struct usb_context_s* MX_USB_DEVICE_Init(void);

void send_sensors(struct usb_context_s *ctx, int16_t current_temperature);
void send_status(struct usb_context_s *ctx, bool TEC, bool fan, int window_heater);
void send_shutter(struct usb_context_s *ctx, bool exposure);

#ifdef __cplusplus
}
#endif
