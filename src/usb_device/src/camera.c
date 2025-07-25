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
static uint8_t *GetUsrStrDescriptor(struct _USBD_HandleTypeDef *pdev, uint8_t index,  uint16_t *length);

size_t USBD_CAMERA_CfgDesc_len;
size_t USBD_CAMERA_HID_Report_len;

extern USBD_HandleTypeDef hUsbDeviceHS;

__ALIGN_BEGIN uint8_t USBD_CAMERA_CfgDesc[256] __ALIGN_END;
__ALIGN_BEGIN uint8_t USBD_CAMERA_HID_Report[256] __ALIGN_END;
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
        .GetUsrStrDescriptor = GetUsrStrDescriptor,
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
    USBD_CAMERA_handle.dfu_mode = false;
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

uint8_t USBD_CAMERA_Configure_DFU(void)
{
    USBD_CAMERA_handle.dfu_mode = true;
    USBD_CAMERA_CfgDesc_len = camera_generate_descriptor_dfu(USBD_CAMERA_CfgDesc,
                                                             sizeof(USBD_CAMERA_CfgDesc));
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    pdev->pClassDataCmsit[pdev->classId] = (void *)&USBD_CAMERA_handle;
    pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

    DFU_Init(USBD_CAMERA_handle.dfu_mode);
    if (USBD_CAMERA_handle.dfu_mode) {
        // nothing
    } else {
        HID_Init(pdev, cfgidx);
        USBD_CAMERA_handle.VS_alt = 0x00U;
    }

    USBD_CAMERA_handle.ep0rx_iface = -1;
    USBD_CAMERA_handle.classId = pdev->classId;
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    if (!USBD_CAMERA_handle.dfu_mode) {
        if (pdev->ep_in[CAMERA_UVC_EPIN & 0xFU].is_used)
        {
            USBD_LL_CloseEP(pdev, CAMERA_UVC_EPIN);
            pdev->ep_in[CAMERA_UVC_EPIN & 0xFU].is_used = 0U;
        }
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestDirection = req->bmRequest & 0x80;
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    uint8_t requestRecipicient = req->bmRequest & USB_REQ_RECIPIENT_MASK;

    if (USBD_CAMERA_handle.dfu_mode) {
        switch (req->wIndex)
        {
        case CAMERA_DFU_DFU_INTERFACE_ID:
            DFU_Setup(pdev, req);
            break;
        default:
            break;
        }
    } else {
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
        case CAMERA_DFU_RUNTIME_INTERFACE_ID:
            DFU_Setup(pdev, req);
            break;
        default:
            break;
        }
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_SOF(struct _USBD_HandleTypeDef *pdev)
{
    if (!USBD_CAMERA_handle.dfu_mode) {
        VS_SOF(pdev);
    }
    DFU_SOF(pdev);
    return USBD_OK;
}

static uint8_t USBD_CAMERA_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    uint8_t res = USBD_OK;
    if (USBD_CAMERA_handle.dfu_mode) {
        switch (USBD_CAMERA_handle.ep0rx_iface)
        {
        case CAMERA_DFU_DFU_INTERFACE_ID:
            res = DFU_EP0_RxReady(pdev);
            break;
        default:
            res = USBD_FAIL;
            break;
        }
    } else {
        switch (USBD_CAMERA_handle.ep0rx_iface)
        {
        case CAMERA_HID_INTERFACE_ID:
            res = HID_EP0_RxReady(pdev);
            break;
        case CAMERA_VS_INTERFACE_ID:
            break;
        case CAMERA_VC_INTERFACE_ID:
            break;
        case CAMERA_DFU_RUNTIME_INTERFACE_ID:
            res = DFU_EP0_RxReady(pdev);
            break;
        default:
            res = USBD_FAIL;
            break;
        }
    }
    USBD_CAMERA_handle.ep0rx_iface = -1;
    return res;
}

static uint8_t USBD_CAMERA_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (!USBD_CAMERA_handle.dfu_mode)
    {
        switch (EPNUM(epnum))
        {
        case EPNUM(CAMERA_HID_EPIN):
            HID_DataIn(pdev, epnum);
            break;
        case EPNUM(CAMERA_UVC_EPIN):
            VS_DataIn(pdev, epnum);
            break;
        }
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (!USBD_CAMERA_handle.dfu_mode)
    {
        switch (EPNUM(epnum))
        {
        case EPNUM(CAMERA_HID_EPOUT):
            HID_DataOut(pdev, epnum);
            break;
        }
    }
    return (uint8_t)USBD_OK;
}

static uint8_t USBD_CAMERA_IsoINIncomplete(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (!USBD_CAMERA_handle.dfu_mode)
    {
        switch (EPNUM(epnum))
        {
        case EPNUM(CAMERA_UVC_EPIN):
            VS_IsoINIncomplete(pdev, epnum);
            break;
        default:
            break;
        }
    }
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

void USBD_CAMERA_ExpectRx(uint8_t interface)
{
    USBD_CAMERA_handle.ep0rx_iface = interface;
}

uint8_t USBD_CAMERA_RegisterInterface(USBD_HandleTypeDef *pdev, struct USBD_CAMERA_callbacks_t *cbs)
{
    pdev->pUserData[pdev->classId] = cbs;
    return USBD_OK;
}


static uint8_t *GetUsrStrDescriptor(struct _USBD_HandleTypeDef *pdev, uint8_t index,  uint16_t *length)
{
    if (index == 0x30U) {
        __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
            2 + (8+6) * 2,
            USB_DESC_TYPE_STRING,
            'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'n', 0, 'a', 0, 'l', 0,
            ' ', 0, 'f', 0, 'l', 0, 'a', 0, 's', 0, 'h', 0
        };
        *length = sizeof(desc);
        return desc;
    }

    if (index == 0x31U) {
        __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
            2 + 6 * 2,
            USB_DESC_TYPE_STRING,
            'P', 0, 'a', 0, 'r', 0, 'a', 0, 'm', 0, 's', 0
        };
        *length = sizeof(desc);
        return desc;
    }

    if (index == 0x32U) {
        __ALIGN_BEGIN static uint8_t desc[] __ALIGN_END = {
            2 + (4) * 2,
            USB_DESC_TYPE_STRING,
            'F', 0, 'P', 0, 'G', 0, 'A', 0
        };
        *length = sizeof(desc);
        return desc;
    }
    return NULL;
}
