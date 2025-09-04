#include "usb_device.h"
#include "usbd_core.h"
#include <stdint.h>

#include "stm32f4xx_hal_pcd.h"
#include "camera.h"

USBD_HandleTypeDef hUsbDeviceHS;

static struct usb_context_s usb_context;

extern USBD_DescriptorsTypeDef USB_Descriptors;

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}

#define UNSIGNED16(low, high) ((((unsigned)(high)) << 8) | (low))
#define BOOL(x) ((x) != 0)

static uint8_t HID_OutputReport(uint8_t report_id, const uint8_t *data, size_t len, bool from_interrupt)
{
    switch (report_id) {
    case TARGET_TEMPERATURE:
        if (len != 3)
            return USBD_FAIL;
        if (usb_context.set_target_temperature != NULL)
            return usb_context.set_target_temperature(UNSIGNED16(data[1], data[2]));
        return USBD_OK;
    case POWER_CTL:
        if (len != 2)
            return USBD_FAIL;
        if (usb_context.set_power_settings != NULL)
            return usb_context.set_power_settings(BOOL(data[1] & 0x01U), BOOL(data[1] & 0x02U), (data[1] >> 2) & 0x0FU);
        return USBD_OK;
    case EXPOSURE_CTL:
        if (len != 3)
            return USBD_FAIL;
        if (usb_context.exposure != NULL)
            return usb_context.exposure(UNSIGNED16(data[1], data[2]));
        return USBD_OK;
    case EXPOSURE_MODE_CTL:
        if (len != 2)
            return USBD_FAIL;
        if (usb_context.exposure_mode != NULL)
            return usb_context.exposure_mode(data[1] & 0x03U);
        return USBD_OK;
    }
    return USBD_FAIL;
}

static uint8_t *HID_GetInReport(uint8_t report_id, size_t* len)
{
    switch (report_id) {
    case CURRENT_TEMPERATURE:
        *len = sizeof(usb_context.current_temperature_buf);
        return usb_context.current_temperature_buf;
    case POWER_STATUS:
        *len = sizeof(usb_context.power_status_buf);
        return usb_context.power_status_buf;
    case EXPOSURE_STATUS:
        *len = sizeof(usb_context.exposure_status_buf);
        return usb_context.exposure_status_buf;
    default:
        *len = 0;
        return NULL;
    }
}

static uint8_t VS_StartStream(void)
{
    return USBD_OK;
}

static uint8_t VS_StopStream(void)
{
    return USBD_OK;
}

static uint8_t CDC_ACM_Control(uint8_t request, uint8_t *data, size_t len)
{
    memset(data, 0, len);
    return USBD_OK;
}

static uint8_t CDC_DATA_DataOut(const uint8_t *data, size_t len)
{
    return USBD_OK;
}

static struct USBD_CAMERA_callbacks_t callbacks = {
    .VS_StartStream = VS_StartStream,
    .VS_StopStream = VS_StopStream,
    .CDC_ACM_Control = CDC_ACM_Control,
    .CDC_DATA_DataOut = CDC_DATA_DataOut,
};

struct usb_context_s* USB_DEVICE_Init(unsigned fps, unsigned width, unsigned height, const char *FourCC)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    /* Config camera parameters */
    if (USBD_CAMERA_Configure(fps, width, height, FourCC) != USBD_OK)
        return NULL;

    /* Init USB */
    if (USBD_Init(&hUsbDeviceHS, &USB_Descriptors, DEVICE_HS) != USBD_OK)
        return NULL;

    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CAMERA) != USBD_OK)
        return NULL;

    USBD_CAMERA_RegisterInterface(&hUsbDeviceHS, &callbacks);

    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        return NULL;

    return &usb_context;
}

struct usb_context_s* USB_DEVICE_Init_DFU(void)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    /* Config camera parameters */
    USBD_CAMERA_Configure_DFU();

    /* Init USB */
    if (USBD_Init(&hUsbDeviceHS, &USB_Descriptors, DEVICE_HS) != USBD_OK)
        return NULL;

    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_CAMERA) != USBD_OK)
        return NULL;

    USBD_CAMERA_RegisterInterface(&hUsbDeviceHS, &callbacks);

    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        return NULL;

    return &usb_context;
}

uint8_t send_current_temperature(int16_t current_temperature)
{
    return USBD_OK;
}

uint8_t send_power_settings(bool TEC, bool fan, int window_heater)
{
    return USBD_OK;
}

uint8_t send_shutter(bool exposure)
{
    return USBD_OK;
}

uint8_t send_serial_data(const uint8_t *data, size_t len)
{
    if (len == 0)
        return USBD_OK;
    return USBD_CAMERA_CDC_DATA_SendSerial(&hUsbDeviceHS, data, len);
}
