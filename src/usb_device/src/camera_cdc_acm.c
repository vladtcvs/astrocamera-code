#include <stdint.h>
#include <camera_internal.h>

#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"

struct CDC_ACM_State {
    unsigned opcode;
    size_t cmdlen;
    uint32_t data[CAMERA_CDC_ACM_MAX_DATA_SIZE / 4U];
} cdc_acm_state;

uint8_t CDC_ACM_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_StatusTypeDef status;
    status = USBD_LL_OpenEP(pdev, CAMERA_CDC_ACM_EPIN, USBD_EP_TYPE_INTR, CAMERA_CDC_ACM_EPIN_SIZE);
    if (status != USBD_OK)
        return status;

    pdev->ep_in[CAMERA_CDC_ACM_EPIN & 0x0FU].is_used = 1U;
    pdev->ep_in[CAMERA_CDC_ACM_EPIN & 0x0FU].maxpacket = CAMERA_CDC_ACM_EPIN_SIZE;

    UNUSED(cfgidx);
    return USBD_OK;
}

void CDC_ACM_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_CloseEP(pdev, CAMERA_CDC_ACM_EPIN);
    pdev->ep_in[CAMERA_CDC_ACM_EPIN & 0xFU].is_used = 0U;

    UNUSED(cfgidx);
}

uint8_t CDC_ACM_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    return USBD_OK;
}

static void CDC_ACM_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    default:
        break;
    }
}

static void CDC_ACM_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t zero[1] = {0};
    size_t len = MIN(1U, req->wLength);
    USBD_CAMERA_handle.ep0tx_iface = CAMERA_CDC_ACM_INTERFACE_ID;
    USBD_CtlSendData(pdev, zero, len);
}

static void CDC_ACM_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

static void CDC_ACM_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_CDC_ACM_INTERFACE_ID;
        USBD_CtlSendData(pdev, status_info, len);
    }
}

static void CDC_ACM_SetupClass(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestDirection = req->bmRequest & 0x80U;
    uint8_t requestRecipicient = req->bmRequest & USB_REQ_RECIPIENT_MASK;

    struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);

    if (req->wLength != 0U) {
        if (requestDirection) {
            cbs->CDC_ACM_Control(req->bRequest, (uint8_t *)cdc_acm_state.data, req->wLength);
            size_t len = MIN(CAMERA_CDC_ACM_REQ_MAX_DATA_SIZE, req->wLength);
            USBD_CtlSendData(pdev, (uint8_t *)cdc_acm_state.data, len);
        } else {
            cdc_acm_state.opcode = req->bRequest;
            cdc_acm_state.cmdlen = (uint8_t)MIN(req->wLength, USB_MAX_EP0_SIZE);
            USBD_CAMERA_ExpectRx(CAMERA_CDC_ACM_INTERFACE_ID);
            USBD_CtlPrepareRx(pdev, (uint8_t *)cdc_acm_state.data, cdc_acm_state.cmdlen);
        }
    } else {

    }
}

void CDC_ACM_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        CDC_ACM_SetupClass(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            CDC_ACM_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            CDC_ACM_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            CDC_ACM_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            CDC_ACM_SetInterface(pdev, req);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

uint8_t CDC_ACM_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    // Here we receive data after CDC_ACM_SetupClass()
    return USBD_OK;
}
