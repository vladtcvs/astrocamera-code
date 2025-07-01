#include "system.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
//#include "usbd_hid.h"
#include "usbd_video.h"
#include "usbd_customhid.h"
#include "usbd_composite_builder.h"
#include "usbd_customhid_if.h"
#include "usbd_video_if.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;
uint8_t HID_EpAdd_Inst = 0x81;
uint8_t VIDEO_EpAdd_Inst = 0x83;

struct usb_context_s {
    int hidClassId;
    int videoClassId;
} usb_context;

/**
 * Init USB device Library, add supported class and start the library
 * @retval None
 */
struct usb_context_s* MX_USB_DEVICE_Init(void)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
        return NULL;

    usb_context.hidClassId = hUsbDeviceHS.classId;
    if (USBD_RegisterClassComposite(&hUsbDeviceHS, &USBD_CUSTOM_HID, CLASS_TYPE_CHID, &HID_EpAdd_Inst) != USBD_OK)
        return NULL;
    hUsbDeviceHS.pUserData[usb_context.hidClassId] = &USBD_CustomHID_fops;
    
    usb_context.videoClassId = hUsbDeviceHS.classId;
    if (USBD_RegisterClassComposite(&hUsbDeviceHS, &USBD_VIDEO, CLASS_TYPE_VIDEO, &VIDEO_EpAdd_Inst) != USBD_OK)
        return NULL;
    hUsbDeviceHS.pUserData[usb_context.videoClassId] = &USBD_VIDEO_fops_FS;

    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
        return NULL;

    return &usb_context;
}
