#include <stdbool.h>
#include <camera_internal.h>

#include "usbd_core.h"
#include "usbd_def.h"

#include "usbd_conf.h"


static struct {
    uint8_t rxbuf[CAMERA_CDC_DATA_EPOUT_SIZE];
    uint8_t txbuf[CAMERA_CDC_DATA_EPIN_SIZE];
    size_t txbuf_len;
    bool busy;
} cdc_data_state;

uint8_t CDC_DATA_Init(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_StatusTypeDef status;

    status = USBD_LL_OpenEP(pdev, CAMERA_CDC_DATA_EPIN, USBD_EP_TYPE_BULK, CAMERA_CDC_DATA_EPIN_SIZE);
    if (status != USBD_OK)
        return status;

    status = USBD_LL_OpenEP(pdev, CAMERA_CDC_DATA_EPOUT, USBD_EP_TYPE_BULK, CAMERA_CDC_DATA_EPOUT_SIZE);
    if (status != USBD_OK) {
        USBD_LL_CloseEP(pdev, CAMERA_CDC_DATA_EPIN);
        return status;
    }

    pdev->ep_in[CAMERA_CDC_DATA_EPIN & 0x0FU].is_used = 1U;
    pdev->ep_in[CAMERA_CDC_DATA_EPIN & 0x0FU].maxpacket = CAMERA_CDC_DATA_EPIN_SIZE;

    pdev->ep_out[CAMERA_CDC_DATA_EPOUT & 0x0FU].is_used = 1U;
    pdev->ep_out[CAMERA_CDC_DATA_EPOUT & 0x0FU].maxpacket = CAMERA_CDC_DATA_EPOUT_SIZE;

    status = USBD_LL_PrepareReceive(pdev, CAMERA_CDC_DATA_EPOUT, cdc_data_state.rxbuf, CAMERA_CDC_DATA_EPOUT_SIZE);
    if (status != USBD_OK) {
        USBD_LL_CloseEP(pdev, CAMERA_CDC_DATA_EPIN);
        USBD_LL_CloseEP(pdev, CAMERA_CDC_DATA_EPOUT);
        pdev->ep_in[CAMERA_CDC_DATA_EPIN & 0x0FU].is_used = 0U;
        pdev->ep_out[CAMERA_CDC_DATA_EPOUT & 0x0FU].is_used = 0U;
        return status;
    }

    cdc_data_state.busy = false;

    UNUSED(cfgidx);
    return USBD_OK;
}

void CDC_DATA_DeInit(struct _USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    USBD_LL_CloseEP(pdev, CAMERA_CDC_DATA_EPIN);
    pdev->ep_in[CAMERA_CDC_DATA_EPIN & 0xFU].is_used = 0U;

    USBD_LL_CloseEP(pdev, CAMERA_CDC_DATA_EPOUT);
    pdev->ep_out[CAMERA_CDC_DATA_EPOUT & 0xFU].is_used = 0U;

    UNUSED(cfgidx);
}

uint8_t USBD_CAMERA_CDC_DATA_SendSerial(USBD_HandleTypeDef *pdev, const uint8_t *data, size_t len)
{
    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;
    if (len > CAMERA_CDC_DATA_EPIN_SIZE)
        return USBD_FAIL;
    if (cdc_data_state.busy)
        return USBD_BUSY;
    cdc_data_state.busy = true;
    memcpy(cdc_data_state.txbuf, data, len);
    cdc_data_state.txbuf_len = len;
    return USBD_LL_Transmit(pdev, CAMERA_CDC_DATA_EPIN, cdc_data_state.txbuf, cdc_data_state.txbuf_len);
}

void CDC_DATA_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{

}

uint8_t CDC_DATA_DataIn(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    cdc_data_state.busy = false;
    return USBD_OK;
}

uint8_t CDC_DATA_DataOut(struct _USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    uint8_t res = USBD_OK;
    // Receive data
    if (pdev->pUserData[USBD_CAMERA_handle.classId] != NULL) {
        struct USBD_CAMERA_callbacks_t *cbs = (struct USBD_CAMERA_callbacks_t *)(pdev->pUserData[USBD_CAMERA_handle.classId]);
        if (cbs->CDC_DATA_DataOut != NULL) {
            size_t len = USBD_LL_GetRxDataSize(pdev, epnum);
            res = cbs->CDC_DATA_DataOut(cdc_data_state.rxbuf, len);
        }
    }

    USBD_LL_PrepareReceive(pdev, CAMERA_CDC_DATA_EPOUT, cdc_data_state.rxbuf,
                                 CAMERA_CDC_DATA_EPOUT_SIZE);

    return res;
}
