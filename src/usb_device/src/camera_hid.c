#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"
#include <camera_descriptor.h>
#include "camera.h"
#include "camera_internal.h"

#define HID_REPORT 0x22U
#define UVC_CS_DEVICE 0x21U

#define GET_REPORT      0x01U
#define GET_IDLE        0x02U
#define GET_PROTOCOL    0x03U
#define SET_REPORT      0x09U
#define SET_IDLE        0x0AU
#define SET_PROTOCOL    0x0BU

static struct {
    bool busy;
    uint8_t protocol;
    uint8_t idle_rate;
    size_t expect_outreport;
    uint8_t expect_report_id;
    uint8_t outreport_buf[CAMERA_HID_OUTREPORT_BUF_SIZE];
    uint8_t inreport_buf[CAMERA_HID_INREPORT_BUF_SIZE];
    size_t inreport_len;
    uint8_t epout_buf[CAMERA_HID_EPOUT_SIZE];
    int classId;
} hid_state;

static void HID_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case HID_REPORT:
    {
        size_t len = MIN(USBD_CAMERA_HID_Report_len, req->wLength);
        uint8_t *pbuf = USBD_CAMERA_HID_Report;
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;;
    }
}

static void HID_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t zero[1] = {0};
    size_t len = MIN(1U, req->wLength);
    USBD_CtlSendData(pdev, zero, len);
}

static void HID_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

static void HID_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CtlSendData(pdev, status_info, len);
    }
}

static uint8_t HID_GetReport(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    int report_type = HIBYTE(req->wValue);
    int report_id = LOBYTE(req->wValue);

    if (report_type != 0x01U)   // input
        return USBD_FAIL;

    if (pdev->pUserData[hid_state.classId] != NULL) {
        struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[hid_state.classId]);
        if (cbs->HID_GetInReport != NULL) {
            size_t len;
            uint8_t *in_report = cbs->HID_GetInReport(report_id, &len);
            USBD_CtlSendData(pdev, in_report, len);
        }
    }
    return USBD_OK;
}

uint8_t HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    if (pdev->pUserData[hid_state.classId] != NULL) {
        struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[hid_state.classId]);
        if (cbs->HID_OutputReport != NULL) {
            uint8_t res = cbs->HID_OutputReport(hid_state.expect_report_id, hid_state.outreport_buf, hid_state.expect_outreport, false);
            hid_state.expect_outreport = 0;
            hid_state.expect_report_id = 0;
            return res;
        }
    }
    return USBD_OK;
}

uint8_t USBD_CAMERA_HID_SendReport(USBD_HandleTypeDef *pdev, const uint8_t *data, size_t len)
{
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    if (len > CAMERA_HID_INREPORT_BUF_SIZE)
        return USBD_FAIL;
    if (hid_state.busy)
        return USBD_BUSY;
    hid_state.busy = true;
    memcpy(hid_state.inreport_buf, data, len);
    hid_state.inreport_len = len;
    return USBD_LL_Transmit(pdev, CAMERA_HID_EPIN, hid_state.inreport_buf, hid_state.inreport_len);
}

uint8_t HID_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    hid_state.busy = false;
    if (hid_state.inreport_len == 0)
        return USBD_OK;
    return USBD_OK;
}

uint8_t HID_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    if (pdev->pUserData[hid_state.classId] != NULL) {
        struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[hid_state.classId]);
        if (cbs->HID_OutputReport != NULL) {
            uint32_t len = USBD_LL_GetRxDataSize(pdev, epnum);
            return cbs->HID_OutputReport(hid_state.epout_buf[0], hid_state.epout_buf, len, true);
        }
    }
    USBD_LL_PrepareReceive(pdev, 0x01, hid_state.epout_buf, CAMERA_HID_EPOUT_SIZE);
    return USBD_OK;
}

static uint8_t HID_SetReport(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // Host will send data in EP0 OUT; handle in EP0_RxReady

    int report_type = HIBYTE(req->wValue);
    int report_id = LOBYTE(req->wValue);

    if (report_type != 0x02U)   // output
        return USBD_FAIL;

    if (req->wLength > CAMERA_HID_OUTREPORT_BUF_SIZE)
    {
        return USBD_FAIL;
    }
    hid_state.expect_outreport = req->wLength;
    hid_state.expect_report_id = report_id;
    USBD_CAMERA_ExpectRx(CAMERA_HID_INTERFACE_ID);
    USBD_CtlPrepareRx(pdev, hid_state.outreport_buf, req->wLength);
    return USBD_OK;
}

static uint8_t HID_SetupClass(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestDirection = req->bmRequest & 0x80U;
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    uint8_t requestRecipicient = req->bmRequest & USB_REQ_RECIPIENT_MASK;

    switch (req->bRequest)
    {
    case GET_REPORT:
        return HID_GetReport(pdev, req);

    case SET_REPORT:
        return HID_SetReport(pdev, req);

    case GET_IDLE:
        USBD_CtlSendData(pdev, &hid_state.idle_rate, 1);
        break;

    case GET_PROTOCOL:
        USBD_CtlSendData(pdev, &hid_state.protocol, 1);
        break;

    case SET_IDLE:
        hid_state.idle_rate = (req->wValue >> 8);
        USBD_CtlSendStatus(pdev);
        break;

    case SET_PROTOCOL:
        hid_state.protocol = (uint8_t)(req->wValue);
        break;

    default:
        return USBD_FAIL;
    }

    return USBD_OK;
}

void HID_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        HID_SetupClass(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            HID_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            HID_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            HID_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            HID_SetInterface(pdev, req);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

uint8_t HID_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_OpenEP(pdev, CAMERA_HID_EPIN, USBD_EP_TYPE_INTR, CAMERA_HID_EPIN_SIZE);
    pdev->ep_in[CAMERA_HID_EPIN & 0x0FU].is_used = 1U;
    pdev->ep_in[CAMERA_HID_EPIN & 0x0FU].maxpacket = CAMERA_HID_EPIN_SIZE;

    USBD_LL_OpenEP(pdev, CAMERA_HID_EPOUT, USBD_EP_TYPE_INTR, CAMERA_HID_EPOUT_SIZE);
    pdev->ep_out[CAMERA_HID_EPOUT & 0x0FU].is_used = 1U;
    pdev->ep_out[CAMERA_HID_EPOUT & 0x0FU].maxpacket = CAMERA_HID_EPOUT_SIZE;

    USBD_LL_PrepareReceive(pdev, 0x01, hid_state.epout_buf, CAMERA_HID_EPOUT_SIZE);
    hid_state.busy = false;
    hid_state.classId = pdev->classId;

    UNUSED(cfgidx);
    return USBD_OK;
}

void HID_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_CloseEP(pdev, CAMERA_HID_EPIN);
    pdev->ep_in[CAMERA_HID_EPIN & 0xFU].is_used = 0U;

    USBD_LL_CloseEP(pdev, CAMERA_HID_EPOUT);
    pdev->ep_out[CAMERA_HID_EPOUT & 0xFU].is_used = 0U;

    UNUSED(cfgidx);
}
