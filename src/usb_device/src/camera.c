#include "camera.h"
#include "camera_internal.h"
#include <usbd_core.h>
#include <usbd_def.h>

#include <stdint.h>
#include <stdbool.h>
#include <camera_descriptor.h>

static uint8_t USBD_CAMERA_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CAMERA_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CAMERA_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CAMERA_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CAMERA_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CAMERA_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CAMERA_SOF(struct _USBD_HandleTypeDef *pdev);
static uint8_t USBD_CAMERA_IsoINIncomplete(struct _USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t *USBD_CAMERA_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_CAMERA_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_CAMERA_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CAMERA_GetDeviceQualifierDesc(uint16_t *length);

size_t USBD_CAMERA_CfgDesc_len;
size_t USBD_CAMERA_HID_Report_len;

extern USBD_HandleTypeDef hUsbDeviceHS;

__ALIGN_BEGIN uint8_t USBD_CAMERA_CfgDesc[1024] __ALIGN_END;
__ALIGN_BEGIN uint8_t USBD_CAMERA_HID_Report[512] __ALIGN_END;
__ALIGN_BEGIN uint8_t video_Probe_Control[48] __ALIGN_END;
__ALIGN_BEGIN uint8_t video_Commit_Control[48] __ALIGN_END;

__ALIGN_BEGIN static uint8_t USBD_CAMERA_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
    {
        USB_LEN_DEV_QUALIFIER_DESC,
        USB_DESC_TYPE_DEVICE_QUALIFIER,
        0x00,
        0x02,
        0xEF,
        0x02,
        0x01,
        0x40,
        0x01,
        0x00,
};

USBD_ClassTypeDef USBD_CAMERA =
    {
        .Init = USBD_CAMERA_Init,
        .DeInit = USBD_CAMERA_DeInit,
        .Setup = USBD_CAMERA_Setup,
        .EP0_TxSent = NULL,
        .EP0_RxReady = USBD_CAMERA_EP0_RxReady,
        .DataIn = USBD_CAMERA_DataIn,
        .DataOut = USBD_CAMERA_DataOut,
        .SOF = USBD_CAMERA_SOF,
        .IsoINIncomplete = USBD_CAMERA_IsoINIncomplete,
        .IsoOUTIncomplete = NULL,
        .GetHSConfigDescriptor = USBD_CAMERA_GetHSCfgDesc,
        .GetHSConfigDescriptor = USBD_CAMERA_GetFSCfgDesc,
        .GetOtherSpeedConfigDescriptor = USBD_CAMERA_GetOtherSpeedCfgDesc,
        .GetDeviceQualifierDescriptor = USBD_CAMERA_GetDeviceQualifierDesc,
};

static struct
{
    uint8_t fps;
    uint16_t width;
    uint16_t height;
    char FourCC[4];
} USBD_CAMERA_Config;

struct USBD_CAMERA_handle_t USBD_CAMERA_handle;

uint8_t USBD_CAMERA_Configure(unsigned fps, unsigned width, unsigned height, const char *FourCC)
{
    USBD_CAMERA_Config.fps = fps;
    USBD_CAMERA_Config.width = width;
    USBD_CAMERA_Config.height = height;
    USBD_CAMERA_Config.FourCC[0] = FourCC[0];
    USBD_CAMERA_Config.FourCC[1] = FourCC[1];
    USBD_CAMERA_Config.FourCC[2] = FourCC[2];
    USBD_CAMERA_Config.FourCC[3] = FourCC[3];
    USBD_CAMERA_CfgDesc_len = camera_generate_descriptor(USBD_CAMERA_CfgDesc,
                                                         USBD_CAMERA_Config.fps,
                                                         USBD_CAMERA_Config.width,
                                                         USBD_CAMERA_Config.height,
                                                         USBD_CAMERA_Config.FourCC,
                                                         sizeof(USBD_CAMERA_CfgDesc));

    USBD_CAMERA_HID_Report_len = camera_hid_report_descriptor(USBD_CAMERA_HID_Report,
                                                              sizeof(USBD_CAMERA_HID_Report));

    camera_fill_probe_control(video_Probe_Control, USBD_CAMERA_Config.width, USBD_CAMERA_Config.height);
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    pdev->pClassDataCmsit[pdev->classId] = (void *)&USBD_CAMERA_handle;
    pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

    if (pdev->dev_speed == USBD_SPEED_HIGH)
    {
        USBD_LL_OpenEP(pdev, CAMERA_UVC_IN_EP, USBD_EP_TYPE_ISOC, CAMERA_UVC_ISO_HS_MPS);

        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].is_used = 1U;
        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_HS_MPS;
    }
    else
    {
        USBD_LL_OpenEP(pdev, CAMERA_UVC_IN_EP, USBD_EP_TYPE_ISOC, CAMERA_UVC_ISO_FS_MPS);

        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].is_used = 1U;
        pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_FS_MPS;
    }

    pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].is_used = 1U;
    pdev->ep_in[CAMERA_UVC_IN_EP & 0x0FU].maxpacket = CAMERA_UVC_ISO_HS_MPS;

    USBD_CAMERA_handle.VS_alt = 0x00U;
    USBD_CAMERA_handle.ep0rx_iface = -1;

    HID_Init(pdev, cfgidx);
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_CloseEP(pdev, CAMERA_UVC_IN_EP);
    pdev->ep_in[CAMERA_UVC_IN_EP & 0xFU].is_used = 0U;

    return (uint8_t)USBD_OK;
}



static uint8_t USBD_CAMERA_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestDirection = req->bmRequest & 0x80;
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    uint8_t requestRecipicient = req->bmRequest & USB_REQ_RECIPIENT_MASK;

    switch (req->wIndex)
    {
    case CAMERA_VS_INTERFACE_ID:
        VS_Setup(pdev, req);
        break;
    case CAMERA_VC_INTERFACE_ID:
        VC_Setup(pdev, req);
        break;
    case CAMERA_HID_INTERFACE_ID:
        HID_Setup(pdev, req);
        break;
    default:
        break;
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    uint8_t res = USBD_OK;
    switch (USBD_CAMERA_handle.ep0rx_iface) {
    case CAMERA_HID_INTERFACE_ID:
        res = HID_EP0_RxReady(pdev);
        break;
    case CAMERA_VS_INTERFACE_ID:
        break;
    case CAMERA_VC_INTERFACE_ID:
        break;
    default:
        return USBD_FAIL;
    }
    USBD_CAMERA_handle.ep0rx_iface = -1;
    return res;
}

static uint8_t USBD_CAMERA_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    switch (epnum) {
    case CAMERA_HID_EPOUT:
        HID_DataOut(pdev, epnum);
        break;
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_SOF(struct _USBD_HandleTypeDef *pdev)
{
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_IsoINIncomplete(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    return (uint8_t)USBD_OK;
}

static uint8_t *USBD_CAMERA_GetHSCfgDesc(uint16_t *length)
{
    *length = USBD_CAMERA_CfgDesc_len;
    return USBD_CAMERA_CfgDesc;
}

static uint8_t *USBD_CAMERA_GetFSCfgDesc(uint16_t *length)
{
    *length = USBD_CAMERA_CfgDesc_len;
    return USBD_CAMERA_CfgDesc;
}

static uint8_t *USBD_CAMERA_GetOtherSpeedCfgDesc(uint16_t *length)
{
    *length = USBD_CAMERA_CfgDesc_len;
    return USBD_CAMERA_CfgDesc;
}

static uint8_t *USBD_CAMERA_GetDeviceQualifierDesc(uint16_t *length)
{
    *length = sizeof(USBD_CAMERA_DeviceQualifierDesc);
    return USBD_CAMERA_DeviceQualifierDesc;
}

uint8_t USBD_CAMERA_SendHIDReport(uint8_t epAddr, uint8_t *data, size_t len)
{
    if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    USBD_LL_Transmit(&hUsbDeviceHS, CAMERA_HID_EPIN, data, len);
    return USBD_OK;
}

void USBD_CAMERA_ExpectRx(uint8_t interface)
{
    USBD_CAMERA_handle.ep0rx_iface = interface;
}
