#include <usbd_def.h>
#include <stdint.h>
#include "usbd_conf.h"
#include "camera_internal.h"

static uint8_t *GetDeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetLangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetSerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetInterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *GetConfigurationStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);

USBD_DescriptorsTypeDef USB_Descriptors =
{
    .GetDeviceDescriptor = GetDeviceDescriptor,
    .GetLangIDStrDescriptor = GetLangIDStrDescriptor,
    .GetManufacturerStrDescriptor = GetManufacturerStrDescriptor,
    .GetProductStrDescriptor = GetProductStrDescriptor,
    .GetSerialStrDescriptor = GetSerialStrDescriptor,
    .GetConfigurationStrDescriptor = GetConfigurationStrDescriptor,
    .GetInterfaceStrDescriptor = GetInterfaceStrDescriptor,
};

__ALIGN_BEGIN static uint8_t device_desc[] __ALIGN_END = {
    0x12U,                      // bLength
    USB_DESC_TYPE_DEVICE,       // bDescriptorType
    WBVAL(0x0200U),             // bcdUSB
    0x00U,                      // bDeviceClass
    0x00U,                      // bDeviceSubClass
    0x00U,                      // bDeviceProtocol
    USB_MAX_EP0_SIZE,           // bMaxPacketSize
    WBVAL(DEVICE_USB_VID),      // idVendor
    WBVAL(DEVICE_USB_PID),      // idProduct
    WBVAL(DEVICE_VERSION),      // bcdDevice
    USBD_IDX_MFC_STR,           // iManufacturer
    USBD_IDX_PRODUCT_STR,       // iProduct
    USBD_IDX_SERIAL_STR,        // iSerialNumber
    USBD_MAX_NUM_CONFIGURATION, // bNumConfigurations
};

static uint8_t *GetDeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    UNUSED(speed);
    *length = sizeof(device_desc);
    return device_desc;
}

static uint8_t *GetLangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        0x04,                   // bLength
        USB_DESC_TYPE_STRING,
        WBVAL(0x0409),
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}

static uint8_t *GetManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        2 + 22*2,               // bLength
        USB_DESC_TYPE_STRING,   // bDescriptorType
        'V', 0, 'l', 0, 'a', 0, 'd', 0,
        'i', 0, 's', 0, 'l', 0, 'a', 0,
        'v', 0, ' ', 0, 'T', 0, 's', 0,
        'e', 0, 'n', 0, 'd', 0, 'r', 0,
        'o', 0, 'v', 0, 's', 0, 'k', 0,
        'i', 0, 'i', 0
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}

static uint8_t *GetProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        2 + 15*2,
        USB_DESC_TYPE_STRING,
        'P', 0, 'i', 0, 'n', 0, 'w', 0,
        'h', 0, 'e', 0, 'e', 0, 'l', 0,
        ' ', 0, 'C', 0, 'a', 0, 'm', 0,
        'e', 0, 'r', 0, 'a', 0
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}

static uint8_t *GetSerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        2 + 4*2,
        USB_DESC_TYPE_STRING,
        '0', 0, '0', 0, '0', 0, '1', 0
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}

static uint8_t *GetConfigurationStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        2 + 13*2,
        USB_DESC_TYPE_STRING,
        'U', 0, 'S', 0, 'B', 0, '2', 0,
        '.', 0, '0', 0, ' ', 0, 'C', 0,
        'a', 0, 'm', 0, 'e', 0, 'r', 0,
        'a', 0
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}

static uint8_t *GetInterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
        2 + 11*2,
        USB_DESC_TYPE_STRING,
        'H', 0, 'I', 0, 'D', 0, ',', 0,
        ' ', 0, 'V', 0, 'C', 0, ',', 0,
        ' ', 0, 'V', 0, 'S', 0
    };
    UNUSED(speed);
    *length = sizeof(desc);
    return desc;
}
