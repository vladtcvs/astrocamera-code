#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"

struct usb_context_s;

/** USB Device initialization function. */
struct usb_context_s* MX_USB_DEVICE_Init(void);

void send_sensors(struct usb_context_s *ctx, int16_t current_temperature);

#ifdef __cplusplus
}
#endif
