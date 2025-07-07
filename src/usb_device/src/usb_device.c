#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include <stdint.h>

#include "camera.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;

struct usb_context_s {
    int hidClassId;
    int videoClassId;
} usb_context;

/**
 * Init USB device Library, add supported class and start the library
 * @retval None
 */
struct usb_context_s* USB_DEVICE_Init(void)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    USBD_CAMERA_Configure(2, 640, 480, "YUY2");

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
        return NULL;

    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CAMERA) != USBD_OK)
        return NULL;

    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        return NULL;

    return &usb_context;
}

void send_sensors(struct usb_context_s *ctx, int16_t current_temperature)
{
    uint8_t report_buf[3];
    report_buf[0] = 1; // report id
    report_buf[1] = LOBYTE(current_temperature);
    report_buf[2] = HIBYTE(current_temperature);
    
}

void send_status(struct usb_context_s *ctx, bool TEC, bool fan, int window_heater)
{
    uint8_t report_buf[2];
    report_buf[0] = 2; // report id
    report_buf[1] = (TEC << 0) | (fan << 1) | ((window_heater & 0x0F) << 2);
    
}

void send_shutter(struct usb_context_s *ctx, bool exposure)
{
    uint8_t report_buf[2];
    report_buf[0] = 3; // report id
    report_buf[1] = (exposure << 0);
    
}
