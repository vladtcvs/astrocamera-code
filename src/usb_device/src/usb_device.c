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

static uint8_t VS_StartStream(void)
{
    return USBD_OK;
}

static uint8_t VS_StopStream(void)
{
    return USBD_OK;
}

static uint8_t VC_GetGain(unsigned *gain)
{
    if (usb_context.get_gain != NULL)
        return usb_context.get_gain(gain);
    else
        *gain = 0;
    return USBD_OK;
}

static uint8_t VC_SetGain(unsigned gain)
{
    if (usb_context.set_gain != NULL)
        return usb_context.set_gain(gain);
    return USBD_OK;
}

static uint8_t VC_GetExposure(uint32_t *exposure)
{
    if (usb_context.get_exposure != NULL)
        return usb_context.get_exposure(exposure);
    else
        *exposure = VC_DEFAULT_EXPOSURE;
    return USBD_OK;
}

static uint8_t VC_SetExposure(uint32_t exposure)
{
    if (usb_context.set_exposure != NULL)
        return usb_context.set_exposure(exposure);
    return USBD_OK;
}

uint8_t VC_GetFan(unsigned *fan)
{
    if (usb_context.get_fan != NULL)
        return usb_context.get_fan(fan);
    else
        *fan = 0;
    return USBD_OK;
}

uint8_t VC_GetTec(unsigned *tec)
{
    if (usb_context.get_tec != NULL)
        return usb_context.get_tec(tec);
    else
        *tec = 0;
    return USBD_OK;
}

uint8_t VC_GetWindowHeater(unsigned *heater)
{
    if (usb_context.get_window_heater != NULL)
        return usb_context.get_window_heater(heater);
    else
        *heater = 0;
    return USBD_OK;
}

uint8_t VC_GetTriggerMode(unsigned *trigger_mode)
{
    if (usb_context.get_trigger_mode != NULL)
        return usb_context.get_trigger_mode(trigger_mode);
    else
        *trigger_mode = 0;
    return USBD_OK;
}

uint8_t VC_GetTargetTemperature(unsigned *temperature)
{
    if (usb_context.get_target_temperature != NULL)
        return usb_context.get_target_temperature(temperature);
    else
        *temperature = 0;
    return USBD_OK;
}

uint8_t VC_GetCurrentTemperature(unsigned *temperature)
{
    if (usb_context.get_current_temperature != NULL)
        return usb_context.get_current_temperature(temperature);
    else
        *temperature = 0;
    return USBD_OK;
}

uint8_t VC_GetWindowTemperature(unsigned *temperature)
{
    if (usb_context.get_window_temperature != NULL)
        return usb_context.get_window_temperature(temperature);
    else
        *temperature = 0;
    return USBD_OK;
}

uint8_t VC_SetFan(unsigned fan)
{
    if (usb_context.set_fan != NULL)
        return usb_context.set_fan(fan);
    return USBD_OK;
}

uint8_t VC_SetTec(unsigned tec)
{
    if (usb_context.set_tec != NULL)
        return usb_context.set_tec(tec);
    return USBD_OK;
}

uint8_t VC_SetWindowHeater(unsigned heater)
{
    if (usb_context.set_window_heater != NULL)
        return usb_context.set_window_heater(heater);
    return USBD_OK;
}

uint8_t VC_SetTriggerMode(unsigned trigger_mode)
{
    if (usb_context.set_trigger_mode != NULL)
        return usb_context.set_trigger_mode(trigger_mode);
    return USBD_OK;
}

uint8_t VC_SetTargetTemperature(unsigned temperature)
{
    if (usb_context.set_target_temperature != NULL)
        return usb_context.set_target_temperature(temperature);
    return USBD_OK;
}

static uint8_t CDC_ACM_Control(uint8_t request, uint8_t *data, size_t len)
{
    memset(data, 0, len);
    return USBD_OK;
}

static uint8_t CDC_DATA_DataOut(const uint8_t *data, size_t len)
{
    if (usb_context.serial_data != NULL)
        return usb_context.serial_data(data, len);
    return USBD_OK;
}

static struct USBD_CAMERA_callbacks_t callbacks = {
    .VS_StartStream = VS_StartStream,
    .VS_StopStream = VS_StopStream,

    .VC_GetGain = VC_GetGain,
    .VC_SetGain = VC_SetGain,
    .VC_GetExposure = VC_GetExposure,
    .VC_SetExposure = VC_SetExposure,

    .VC_GetCurrentTemperature = VC_GetCurrentTemperature,
    .VC_GetTargetTemperature = VC_GetTargetTemperature,
    .VC_GetWindowTemperature = VC_GetWindowTemperature,
    .VC_GetFan = VC_GetFan,
    .VC_GetTec = VC_GetTec,
    .VC_GetWindowHeater = VC_GetWindowHeater,
    .VC_GetTriggerMode = VC_GetTriggerMode,

    .VC_SetTargetTemperature = VC_SetTargetTemperature,
    .VC_SetFan = VC_SetFan,
    .VC_SetTec = VC_SetTec,
    .VC_SetWindowHeater = VC_SetWindowHeater,
    .VC_SetTriggerMode = VC_SetTriggerMode,


    .CDC_ACM_Control = CDC_ACM_Control,
    .CDC_DATA_DataOut = CDC_DATA_DataOut,
};

struct usb_context_s* USB_DEVICE_Init(unsigned fps, unsigned width, unsigned height, const char *FourCC)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
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
