#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_video.h"
#include "usbd_customhid.h"
#include "usbd_composite_builder.h"
#include "usbd_customhid_if.h"
#include "usbd_video_if.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;

static uint8_t HID_EpAdd_Inst[1] = {HID_EPIN_ADDR};
static uint8_t CUSTOM_HID_EpAdd_Inst[2] = {CUSTOM_HID_EPIN_ADDR, CUSTOM_HID_EPOUT_ADDR};
static uint8_t VIDEO_EpAdd_Inst[1] = {UVC_IN_EP};


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

#ifdef USBD_COMPOSITE
    usb_context.hidClassId = hUsbDeviceHS.classId;
    if (USBD_RegisterClassComposite(&hUsbDeviceHS, &USBD_CUSTOM_HID, CLASS_TYPE_CHID, CUSTOM_HID_EpAdd_Inst) != USBD_OK)
        return NULL;
    USBD_CUSTOM_HID_RegisterInterface(&hUsbDeviceHS, &USBD_CustomHID_fops);

    usb_context.videoClassId = hUsbDeviceHS.classId;
    if (USBD_RegisterClassComposite(&hUsbDeviceHS, &USBD_VIDEO, CLASS_TYPE_VIDEO, VIDEO_EpAdd_Inst) != USBD_OK)
        return NULL;
    USBD_VIDEO_RegisterInterface(&hUsbDeviceHS, &USBD_VIDEO_fops_FS);
#else
    usb_context.videoClassId = hUsbDeviceHS.classId;
    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_VIDEO) != USBD_OK)
        return NULL;
    USBD_VIDEO_RegisterInterface(&hUsbDeviceHS, &USBD_VIDEO_fops_FS);
#endif

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
    //USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf), ctx->hidClassId);
}

void send_status(struct usb_context_s *ctx, bool TEC, bool fan, int window_heater)
{
    uint8_t report_buf[2];
    report_buf[0] = 2; // report id
    report_buf[1] = (TEC << 0) | (fan << 1) | ((window_heater & 0x0F) << 2);
    //USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf), ctx->hidClassId);
}

void send_shutter(struct usb_context_s *ctx, bool exposure)
{
    uint8_t report_buf[2];
    report_buf[0] = 3; // report id
    report_buf[1] = (exposure << 0);
    //USBD_CUSTOM_HID_SendReport(&hUsbDeviceHS, report_buf, sizeof(report_buf), ctx->hidClassId);
}
