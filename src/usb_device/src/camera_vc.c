#include "camera_internal.h"
#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"
#include <camera_descriptor.h>

#define UVC_SET_CUR 0x01U
#define UVC_GET_CUR 0x81U
#define UVC_GET_MIN 0x82U
#define UVC_GET_MAX 0x83U
#define UVC_GET_RES 0x84U
#define UVC_GET_LEN 0x85U
#define UVC_GET_INFO 0x86U
#define UVC_GET_DEF 0x87U

#define XU_FAN                  0x01U
#define XU_TEC                  0x02U
#define XU_WINDOW_HEATER        0x03U
#define XU_TARGET_TEMPERATURE   0x04U
#define XU_CURRENT_TEMPERATURE  0x05U
#define XU_WINDOW_TEMPERATURE   0x06U
#define XU_TRIGGER_MODE         0x07U

#define MAX_EXPECTED_TEMPERATURE 10000U // 1000 K in 0.1K
#define MAX_TM 0x03

#define VS_PROBE_CONTROL 0x100U
#define VS_COMMIT_CONTROL 0x200U

struct {
    bool expect_buf;
    uint8_t set_cur_buf[4];
    size_t  set_cur_buf_len;
    uint8_t set_cur_entity;
    uint8_t set_cur_selector;
} vc_state;

static void VC_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case UVC_CS_DEVICE:
    {
        size_t len = 0;
        const void *pbuf = camera_get_video_descriptor(&len);
        len = MIN(len, req->wLength);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;
    }
}

static void VC_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t zero[1] = {0};
    size_t len = MIN(1U, req->wLength);
    USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
    USBD_CtlSendData(pdev, zero, len);
}


static void VC_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    // do nothing
}

static void VC_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, status_info, len);
    }
}

static void VC_Req_GET_DEF(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            const uint32_t exposure = VC_DEFAULT_EXPOSURE; // 0.1 sec
            switch (cs) {
            case 0x04:  // Absolute time
                buf[0] = exposure & 0xFFU;
                buf[1] = (exposure >> 8) & 0xFFU;
                buf[2] = (exposure >> 16) & 0xFFU;
                buf[3] = (exposure >> 24) & 0xFFU;
                len = 4;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                buf[0] = 0x00U;
                len = 1;
                break;
            case XU_TEC:     // TEC
                buf[0] = 0x00U;
                len = 1;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                buf[0] = 0x00U;
                len = 1;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                buf[0] = 0x00;
                len = 1;
                break;
            }
        }
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_MIN(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // Absolute time
                buf[0] = 0x01;
                buf[1] = 0x00;
                buf[2] = 0x00;
                buf[3] = 0x00;
                len = 4;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                buf[0] = 0x00;
                len = 1;
                break;
            case XU_TEC:     // TEC
                buf[0] = 0x00;
                len = 1;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                buf[0] = 0x00;
                len = 1;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                buf[0] = 0x00;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                buf[0] = 0x00;
                len = 1;
                break;
            }
        }
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_MAX(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            const uint32_t max_exposure = 10000*3600; // 1 hour
            switch (cs) {
            case 0x04:  // Absolute time
                buf[0] = max_exposure & 0xFFU;
                buf[1] = (max_exposure >> 8) & 0xFFU;
                buf[2] = (max_exposure >> 16) & 0xFFU;
                buf[3] = (max_exposure >> 24) & 0xFFU;
                len = 4;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0xFF;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                buf[0] = 0x01U;
                len = 1;
                break;
            case XU_TEC:     // TEC
                buf[0] = 0x01U;
                len = 1;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                buf[0] = 0xFFU;
                len = 1;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                buf[0] = LOBYTE(MAX_EXPECTED_TEMPERATURE);
                buf[1] = HIBYTE(MAX_EXPECTED_TEMPERATURE);
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                buf[0] = LOBYTE(MAX_EXPECTED_TEMPERATURE);
                buf[1] = HIBYTE(MAX_EXPECTED_TEMPERATURE);
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                buf[0] = LOBYTE(MAX_EXPECTED_TEMPERATURE);
                buf[1] = HIBYTE(MAX_EXPECTED_TEMPERATURE);
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                buf[0] = MAX_TM;
                len = 1;
                break;
            }
        }
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_RES(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // Absolute time
                buf[0] = 0x01;
                buf[1] = 0x00;
                buf[2] = 0x00;
                buf[3] = 0x00;
                len = 4;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                buf[0] = 0x01;
                buf[1] = 0x00;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                buf[0] = 0x01U;
                len = 1;
                break;
            case XU_TEC:     // TEC
                buf[0] = 0x01U;
                len = 1;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                buf[0] = 0x01U;
                len = 1;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                buf[0] = 0x01U;
                buf[1] = 0x00U;
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                buf[0] = 0x01U;
                buf[1] = 0x00U;
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                buf[0] = 0x01;
                buf[1] = 0x00;
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                buf[0] = 0x01;
                len = 1;
                break;
            }
        }
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_GET_LEN(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4];
    size_t len = 0;
    unsigned ctl_len;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // Absolute time
                ctl_len = 4;
                len = 2;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                ctl_len = 2;
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                ctl_len = 1;
                len = 2;
                break;
            case XU_TEC:     // TEC
                ctl_len = 1;
                len = 2;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                ctl_len = 1;
                len = 2;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                ctl_len = 2;
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                ctl_len = 2;
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                ctl_len = 2;
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                ctl_len = 1;
                len = 2;
                break;
            }
        }
        break;
    }
    
    if (len == 2) {
        buf[0] = LOBYTE(ctl_len);
        buf[1] = HIBYTE(ctl_len);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}


static void VC_Req_GET_INFO(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t caps = 0x00U;
    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // Absolute exposure time
                caps = 0x03U;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                caps = 0x03U;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                caps = 0x03U;
                break;
            case XU_TEC:     // TEC
                caps = 0x03U;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                caps = 0x03U;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                caps = 0x03U;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                caps = 0x01U;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                caps = 0x01U;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                caps = 0x03U;
                break;
            }
        }
        break;
    }
    USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
    USBD_CtlSendData(pdev, &caps, 1);
}

static void VC_Req_GET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);

    uint8_t entity = HIBYTE(req->wIndex);
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t buf[4] = {0};
    size_t len = 0;
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // Absolute exposure time
                if (cbs != NULL && cbs->VC_GetExposure != NULL) {
                    uint32_t time;
                    cbs->VC_GetExposure(&time);
                    buf[0] = time & 0xFFU;
                    buf[1] = (time >> 8) & 0xFFU;
                    buf[2] = (time >> 16) & 0xFFU;
                    buf[3] = (time >> 24) & 0xFFU;
                } else {
                    buf[0] = VC_DEFAULT_EXPOSURE & 0xFFU;
                    buf[1] = (VC_DEFAULT_EXPOSURE >> 8) & 0xFFU;
                    buf[2] = (VC_DEFAULT_EXPOSURE >> 16) & 0xFFU;
                    buf[3] = (VC_DEFAULT_EXPOSURE >> 24) & 0xFFU;
                }
                len = 4;
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                if (cbs != NULL && cbs->VC_GetGain != NULL) {
                    unsigned gain;
                    cbs->VC_GetGain(&gain);
                    buf[0] = MIN(gain, 0xFFU) & 0xFFU;
                    buf[1] = 0x00U;
                } else {
                    buf[0] = 0x00U;
                    buf[1] = 0x00U;
                }
                len = 2;
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:     // FAN
                if (cbs != NULL && cbs->VC_GetFan != NULL) {
                    unsigned fan;
                    cbs->VC_GetFan(&fan);
                    buf[0] = MIN(fan, 0x01U);
                } else {
                    buf[0] = 0x00U;
                }
                len = 1;
                break;
            case XU_TEC:     // TEC
                if (cbs != NULL && cbs->VC_GetTec != NULL) {
                    unsigned tec;
                    cbs->VC_GetTec(&tec);
                    buf[0] = MIN(tec, 0x01U);
                } else {
                    buf[0] = 0x00U;
                }
                len = 1;
                break;
            case XU_WINDOW_HEATER:     // WINDOW HEATER
                if (cbs != NULL && cbs->VC_GetWindowHeater != NULL) {
                    unsigned wh;
                    cbs->VC_GetWindowHeater(&wh);
                    buf[0] = MIN(wh, 0xFFU);
                } else {
                    buf[0] = 0x00U;
                }
                len = 1;
                break;
            case XU_TARGET_TEMPERATURE:     // TARGET TEMPERATURE
                if (cbs != NULL && cbs->VC_GetTargetTemperature != NULL) {
                    unsigned tt;
                    cbs->VC_GetTargetTemperature(&tt);
                    buf[0] = LOBYTE(tt);
                    buf[1] = LOBYTE(tt);
                } else {
                    buf[0] = 0x00U;
                    buf[1] = 0x00;
                }
                len = 2;
                break;
            case XU_CURRENT_TEMPERATURE:     // CURRENT TEMPERATURE
                if (cbs != NULL && cbs->VC_GetCurrentTemperature != NULL) {
                    unsigned ct;
                    cbs->VC_GetCurrentTemperature(&ct);
                    buf[0] = LOBYTE(ct);
                    buf[1] = HIBYTE(ct);
                } else {
                    buf[0] = 0x00U;
                    buf[1] = 0x00;
                }
                len = 2;
                break;
            case XU_WINDOW_TEMPERATURE:     // WINDOW TEMPERATURE
                if (cbs != NULL && cbs->VC_GetWindowTemperature != NULL) {
                    unsigned wt;
                    cbs->VC_GetWindowTemperature(&wt);
                    buf[0] = LOBYTE(wt);
                    buf[1] = HIBYTE(wt);
                } else {
                    buf[0] = 0x00U;
                    buf[1] = 0x00;
                }
                len = 2;
                break;
            case XU_TRIGGER_MODE:     // TRIGGER MODE
                if (cbs != NULL && cbs->VC_GetTriggerMode != NULL) {
                    unsigned tm;
                    cbs->VC_GetTriggerMode(&tm);
                    buf[0] = MIN(tm, MAX_TM);
                } else {
                    buf[0] = 0x00U;
                }
                len = 1;
                break;
            }
        }
        break;
    }
    if (len > 0) {
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_VC_INTERFACE_ID;
        USBD_CtlSendData(pdev, buf, len);
    } else {
        USBD_CtlError(pdev, req);
    }
}

static void VC_Req_SET_CUR(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t cs = HIBYTE(req->wValue);
    uint8_t entity = HIBYTE(req->wIndex);
    switch (entity) {
    case 0x01U: // Input terminal
        {
            switch (cs) {
            case 0x04:  // absolute time
                vc_state.expect_buf = true;
                vc_state.set_cur_buf_len = 4;
                vc_state.set_cur_entity = 0x01U;
                vc_state.set_cur_selector = 0x04U;  // absolute time
                USBD_CAMERA_ExpectRx(CAMERA_VC_INTERFACE_ID);
                USBD_CtlPrepareRx(pdev, vc_state.set_cur_buf, 4);
                break;
            default:
                USBD_LL_StallEP(pdev, 0x80U);
                break;
            }
        }
        break;
    case 0x02U: // Processing terminal
        {
            switch (cs) {
            case 0x04:  // Gain
                vc_state.expect_buf = true;
                vc_state.set_cur_buf_len = 2;
                vc_state.set_cur_entity = 0x02U;
                vc_state.set_cur_selector = 0x04U;
                USBD_CAMERA_ExpectRx(CAMERA_VC_INTERFACE_ID);
                USBD_CtlPrepareRx(pdev, vc_state.set_cur_buf, 2);
                break;
            default:
                USBD_LL_StallEP(pdev, 0x80U);
                break;
            }
        }
        break;
    case 0x03U: // Output terminal
        break;
    case 0x04U: // XU
        {
            switch (cs) {
            case XU_FAN:
            case XU_TEC:
            case XU_WINDOW_HEATER:
            case XU_TRIGGER_MODE:
                vc_state.expect_buf = true;
                vc_state.set_cur_buf_len = 1;
                vc_state.set_cur_entity = 0x04U;
                vc_state.set_cur_selector = cs;
                USBD_CAMERA_ExpectRx(CAMERA_VC_INTERFACE_ID);
                USBD_CtlPrepareRx(pdev, vc_state.set_cur_buf, 1);
                break;
            case XU_TARGET_TEMPERATURE:
                vc_state.expect_buf = true;
                vc_state.set_cur_buf_len = 2;
                vc_state.set_cur_entity = 0x04U;
                vc_state.set_cur_selector = cs;
                USBD_CAMERA_ExpectRx(CAMERA_VC_INTERFACE_ID);
                USBD_CtlPrepareRx(pdev, vc_state.set_cur_buf, 2);
                break;
            default:
                USBD_LL_StallEP(pdev, 0x80U);
                break;
            }
        }
        break;
    }
}

uint8_t VC_EP0_RxReady(struct _USBD_HandleTypeDef *pdev)
{
    if (!vc_state.expect_buf)
        return USBD_FAIL;
    
    struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);
    switch (vc_state.set_cur_entity)
    {
        case 0x01U: // Camera terminal
            switch (vc_state.set_cur_selector)
            {
                case 0x04U:  // Exposure
                    {
                        uint32_t gain = vc_state.set_cur_buf[3];
                        gain = gain << 8 | vc_state.set_cur_buf[2];
                        gain = gain << 8 | vc_state.set_cur_buf[1];
                        gain = gain << 8 | vc_state.set_cur_buf[0];

                        if (cbs != NULL && cbs->VC_SetExposure != NULL) {
                            cbs->VC_SetExposure(gain);
                        }
                    }
                    break;
            }
            break;
        case 0x02U: // Processing terminal
            switch (vc_state.set_cur_selector)
            {
                case 0x04U:  // Gain
                    {
                        unsigned gain = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetGain != NULL) {
                            cbs->VC_SetGain(gain);
                        }
                    }
                    break;
            }
            break;
        case 0x03U: // Output terminal
            break;
        case 0x04U: // XU
            switch (vc_state.set_cur_selector)
            {
                case XU_FAN:
                    {
                        unsigned fan = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetFan != NULL) {
                            cbs->VC_SetFan(fan);
                        }
                    }
                    break;
                case XU_TEC:
                    {
                        unsigned tec = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetTec != NULL) {
                            cbs->VC_SetTec(tec);
                        }
                    }
                    break;
                case XU_WINDOW_HEATER:
                    {
                        unsigned wh = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetWindowHeater != NULL) {
                            cbs->VC_SetWindowHeater(wh);
                        }
                    }
                    break;
                case XU_TRIGGER_MODE:
                    {
                        unsigned tm = vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetTriggerMode != NULL) {
                            cbs->VC_SetTriggerMode(tm);
                        }
                    }
                    break;
                case XU_TARGET_TEMPERATURE:
                    {
                        unsigned tt = vc_state.set_cur_buf[1];
                        tt = tt << 8 | vc_state.set_cur_buf[0];
                        if (cbs != NULL && cbs->VC_SetWindowHeater != NULL) {
                            cbs->VC_SetWindowHeater(tt);
                        }
                    }
                    break;
            }
            break;
            
    }
    vc_state.expect_buf = false;
    return USBD_OK;
}

static void VC_SetupClass(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->bRequest)
    {
    case UVC_GET_DEF:
        VC_Req_GET_DEF(pdev, req);
        break;
    case UVC_GET_CUR:
        VC_Req_GET_CUR(pdev, req);
        break;
    case UVC_GET_MIN:
        VC_Req_GET_MIN(pdev, req);
        break;
    case UVC_GET_MAX:
        VC_Req_GET_MAX(pdev, req);
        break;
    case UVC_GET_RES:
        VC_Req_GET_RES(pdev, req);
        break;
    case UVC_GET_LEN:
        VC_Req_GET_LEN(pdev, req);
        break;
    case UVC_GET_INFO:
        VC_Req_GET_INFO(pdev, req);
        break;
    case UVC_SET_CUR:
        VC_Req_SET_CUR(pdev, req);
        break;

    default:
        break;
    }
}

void VC_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        VC_SetupClass(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            VC_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            VC_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            VC_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            VC_SetInterface(pdev, req);
            break;
        default:
            break;
        }

        break;
    default:
        break;
    }
}
