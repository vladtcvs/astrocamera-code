#include "usb_device.h"
#include "usbd_core.h"
#include <stdint.h>

#include "stm32f4xx_hal_pcd.h"
#include "camera.h"

USBD_HandleTypeDef hUsbDeviceHS;

extern USBD_DescriptorsTypeDef USB_Descriptors;

struct usb_context_s {
    
} usb_context;

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}

struct usb_context_s* USB_DEVICE_Init(unsigned fps, unsigned width, unsigned height, const char *FourCC)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    /* Config camera parameters */
    USBD_CAMERA_Configure(fps, width, height, FourCC);

    /* Init USB */
    if (USBD_Init(&hUsbDeviceHS, &USB_Descriptors, DEVICE_HS) != USBD_OK)
        return NULL;

    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CAMERA) != USBD_OK)
        return NULL;

    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        return NULL;

    return &usb_context;
}

uint8_t send_sensors(struct usb_context_s *ctx, int16_t current_temperature)
{
    uint8_t report_buf[3];
    report_buf[0] = 1; // report id
    report_buf[1] = LOBYTE(current_temperature);
    report_buf[2] = HIBYTE(current_temperature);
    USBD_CAMERA_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf));
}

uint8_t send_status(struct usb_context_s *ctx, bool TEC, bool fan, int window_heater)
{
    uint8_t report_buf[2];
    report_buf[0] = 2; // report id
    report_buf[1] = (TEC << 0) | (fan << 1) | ((window_heater & 0x0F) << 2);
    USBD_CAMERA_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf));
}

uint8_t send_shutter(struct usb_context_s *ctx, bool exposure)
{
    uint8_t report_buf[2];
    report_buf[0] = 3; // report id
    report_buf[1] = (exposure << 0);
    return USBD_CAMERA_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf));
}
