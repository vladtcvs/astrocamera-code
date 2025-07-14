#include "camera_internal.h"
#include "camera_descriptor.h"
#include "usbd_core.h"
#include "usbd_def.h"

#include <string.h>

#define DFU_DETACH                     0x00
#define DFU_DNLOAD                     0x01
#define DFU_UPLOAD                     0x02
#define DFU_GETSTATUS                  0x03
#define DFU_CLRSTATUS                  0x04
#define DFU_GETSTATE                   0x05
#define DFU_ABORT                      0x06


enum dfu_status_e {
    DFU_STATUS_OK                = 0x00,  // No error condition is present
    DFU_STATUS_errTARGET         = 0x01,  // File is not intended for this device
    DFU_STATUS_errFILE           = 0x02,  // File is for this device but has errors
    DFU_STATUS_errWRITE          = 0x03,  // Error writing memory
    DFU_STATUS_errERASE          = 0x04,  // Error erasing memory
    DFU_STATUS_errCHECK_ERASED   = 0x05,  // Memory not blank when expected
    DFU_STATUS_errPROG           = 0x06,  // Programming failed
    DFU_STATUS_errVERIFY         = 0x07,  // Verification failed
    DFU_STATUS_errADDRESS        = 0x08,  // Address is out of range
    DFU_STATUS_errNOTDONE        = 0x09,  // Download operation not complete
    DFU_STATUS_errFIRMWARE       = 0x0A,  // Firmware is invalid or corrupt
    DFU_STATUS_errVENDOR         = 0x0B,  // Vendor-specific error
    DFU_STATUS_errUSBR           = 0x0C,  // Unexpected USB reset
    DFU_STATUS_errPOR            = 0x0D,  // Power-on reset occurred
    DFU_STATUS_errUNKNOWN        = 0x0E,  // Unknown error
    DFU_STATUS_errSTALLEDPKT     = 0x0F   // Host sent a stalled packet
};

enum dfu_state_e {
    APP_IDLE                   = 0x00,  // Device is running application, no DFU requests yet
    APP_DETACH                 = 0x01,  // Host sent DFU_DETACH, waiting for reset (runtime mode)

    DFU_IDLE                   = 0x02,  // Device is in DFU mode, ready for DNLOAD/UPLOAD
    DFU_DNLOAD_SYNC            = 0x03,  // Just received block, waiting to process (flash, EEPROM)
    DFU_DNBUSY                 = 0x04,  // Flashing is in progress (internal write, erase, etc)
    DFU_DNLOAD_IDLE            = 0x05,  // Ready for next block (intermediate state)

    DFU_MANIFEST_SYNC          = 0x06,  // Received all blocks, waiting to begin final write
    DFU_MANIFEST               = 0x07,  // Manifestation (writing to flash, or jump)
    DFU_MANIFEST_WAIT_RESET    = 0x08,  // Finished, waiting for host to reset (only if not tolerant)

    DFU_UPLOAD_IDLE            = 0x09,  // Upload in progress
    DFU_ERROR                  = 0x0A   // Error occurred, must be cleared with DFU_CLRSTATUS
};

static struct {
    enum dfu_status_e status;
    enum dfu_state_e state;
    uint8_t DFU_alt;
    uint8_t status_buf[6];
    uint8_t data_buf[DFU_TRANSFER_SIZE];

    uint32_t address;
    uint16_t block_num;
    uint16_t req_len;

    uint8_t delay_cnt;
    bool dfu_mode;
    bool reboot_in_app;
    bool reboot_in_dfu;
} dfu_state;

static void dfu_fill_status_response(uint8_t *buf)
{
    buf[0] = DFU_STATUS_OK;
    buf[1] = 0x00U;
    buf[2] = 0x00U;
    buf[3] = 0x00U;
    buf[4] = dfu_state.state;
    buf[5] = 0x00U;
    switch (dfu_state.state)
    {
    case DFU_DNLOAD_SYNC:
        dfu_state.state = DFU_DNLOAD_IDLE;
        break;
    case DFU_MANIFEST_SYNC:
        dfu_state.state = DFU_MANIFEST;
        break;
    case DFU_MANIFEST:
        dfu_state.state = DFU_MANIFEST_WAIT_RESET;
        break;
    case DFU_MANIFEST_WAIT_RESET:
        dfu_state.delay_cnt = 10;
        dfu_state.reboot_in_app = true;
        break;
    }
    
}

HAL_StatusTypeDef I2C_EEPROM_Write(uint16_t MemAddress, const uint8_t *pData);
HAL_StatusTypeDef I2C_EEPROM_Read(uint16_t MemAddress, uint8_t *pData);

static size_t dfu_fill_buffer(uint8_t *buf, uint16_t block_num, size_t len)
{
    memset(buf, 0xFFU, len);
    switch (dfu_state.DFU_alt)
    {
    // Internal flash
    case 0:
        return 0;

    // Parameters
    case 1:
        {
            size_t size = 0;
            unsigned i;
            for (i = 0; i < len; i += 1) {
                unsigned addr = block_num * DFU_TRANSFER_SIZE + i;
                if (addr >= I2C_EEPROM_SIZE)
                    break;
                int res = I2C_EEPROM_Read(addr, buf + i);
                size += 1;
            }
            return size;
        }

    // FPGA
    case 2:
        return 0;
    }
    return 0;
}

static void dfu_finish_download(void)
{

}

static uint8_t dfu_process_download(const uint8_t *buf, uint16_t block_num, size_t len)
{
    dfu_state.state = DFU_DNLOAD_SYNC;
    switch (dfu_state.DFU_alt)
    {
    // Internal flash
    case 0:
        break;
    // Parameters
    case 1:
        {
            unsigned i;
            for (i = 0; i < len; i += 1) {
                unsigned addr = block_num * DFU_TRANSFER_SIZE + i;
                I2C_EEPROM_Write(addr, buf + i);
            }
        }
        break;
    // FPGA
    case 2:
        break;
    }
    return USBD_OK;
}

void reboot_in_app(void);
void reboot_in_dfu(void);

uint8_t DFU_SOF(USBD_HandleTypeDef *pdev)
{
    if (dfu_state.delay_cnt == 0)
        return USBD_OK;
    dfu_state.delay_cnt--;
    if (dfu_state.delay_cnt > 0)
        return USBD_OK;

    if (USBD_CAMERA_handle.dfu_mode == true) {
        if (dfu_state.reboot_in_app) {
            reboot_in_app();
        }
    } else {
        if (dfu_state.state == APP_DETACH && dfu_state.reboot_in_dfu) {
            reboot_in_dfu();
        }
    }
    return USBD_OK;
}

uint8_t DFU_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    dfu_process_download(dfu_state.data_buf, dfu_state.block_num, dfu_state.req_len);
    return USBD_OK;
}

uint8_t DFU_SetupClass_APP(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->bRequest)
    {
    case DFU_DETACH: // 0x00
        dfu_state.state = APP_DETACH;
        dfu_state.reboot_in_dfu = true;
        USBD_CtlSendStatus(pdev);
        dfu_state.delay_cnt = 10;
        return USBD_OK;
    case DFU_GETSTATUS: // 0x03
        dfu_fill_status_response(dfu_state.status_buf);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_DFU_RUNTIME_INTERFACE_ID;
        USBD_CtlSendData(pdev, dfu_state.status_buf, sizeof(dfu_state.status_buf));
        return USBD_OK;
    default:
        return USBD_OK;
    }
}

uint8_t DFU_SetupClass_DFU(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    switch (req->bRequest)
    {
    case DFU_DETACH: // 0x00
        USBD_CtlSendStatus(pdev);
        dfu_state.delay_cnt = 10;
        return USBD_OK;

    case DFU_DNLOAD: // 0x01
        if (req->wLength > 0)
        {
            // Prepare to receive firmware data on EP0 OUT
            USBD_CAMERA_handle.ep0rx_iface = CAMERA_DFU_DFU_INTERFACE_ID;
            USBD_CtlPrepareRx(pdev, dfu_state.data_buf, req->wLength);
            dfu_state.block_num = req->wValue;
            dfu_state.req_len = req->wLength;
        }
        else
        {
            // Zero-length DNLOAD = finish
            dfu_finish_download();
            dfu_state.state = DFU_MANIFEST_SYNC;
        }
        return USBD_OK;

    case DFU_UPLOAD: // 0x02
        {
            uint16_t block_num = req->wValue;
            uint16_t req_len = req->wLength;
            size_t block_len = dfu_fill_buffer(dfu_state.data_buf, block_num, req_len);
            USBD_CAMERA_handle.ep0tx_iface = CAMERA_DFU_DFU_INTERFACE_ID;
            USBD_CtlSendData(pdev, dfu_state.data_buf, block_len);
            dfu_state.state = DFU_IDLE;
        }
        return USBD_OK;

    case DFU_GETSTATUS: // 0x03
        dfu_fill_status_response(dfu_state.status_buf);
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_DFU_DFU_INTERFACE_ID;
        USBD_CtlSendData(pdev, dfu_state.status_buf, sizeof(dfu_state.status_buf));
        return USBD_OK;

    case DFU_CLRSTATUS: // 0x04
        dfu_state.status = DFU_STATUS_OK;
        dfu_state.state = DFU_IDLE;
        return USBD_OK;

    case DFU_GETSTATE: // 0x05
        USBD_CAMERA_handle.ep0tx_iface = CAMERA_DFU_DFU_INTERFACE_ID;
        USBD_CtlSendData(pdev, (uint8_t *)&dfu_state.state, 1);
        return USBD_OK;

    case DFU_ABORT: // 0x06
        dfu_state.state = DFU_IDLE;
        dfu_state.status = DFU_STATUS_OK;
        dfu_state.block_num = 0;
        USBD_CtlSendStatus(pdev);
        return USBD_OK;

    default:
        return USBD_FAIL;
    }
}

static void DFU_GetStatus(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
    {
        uint8_t status_info[2] = {0, 0};
        size_t len = MIN(2U, req->wLength);
        USBD_CtlSendData(pdev, status_info, len);
    }
}

static void DFU_GetDescriptor(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t descType = HIBYTE(req->wValue);
    switch (descType)
    {
    case DFU_CS_DEVICE:
    {
        size_t len = 0;
        void *pbuf = camera_get_dfu_descriptor(&len);
        len = MIN(len, req->wLength);
        USBD_CtlSendData(pdev, pbuf, len);
        break;
    }
    default:
        break;
    }
}

static uint8_t DFU_SetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t newAlt = LOBYTE(req->wValue);
    if (dfu_state.dfu_mode)
    {
        uint8_t old_alt = dfu_state.DFU_alt;
        if (newAlt > 2)
            return USBD_FAIL;
        if (old_alt == newAlt)
            return USBD_OK;
        dfu_state.DFU_alt = newAlt;
        dfu_state.state = DFU_IDLE;
    } else {
        if (newAlt > 0)
            return USBD_FAIL;
        dfu_state.state = APP_IDLE;
        dfu_state.DFU_alt = 0;
    }
    return USBD_OK;
}

static void DFU_GetInterface(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    size_t len = MIN(1U, req->wLength);
    if (pdev->dev_state == USBD_STATE_CONFIGURED)
        USBD_CtlSendData(pdev, &dfu_state.DFU_alt, len);
}

void DFU_Init(bool dfu_mode)
{
    dfu_state.dfu_mode = dfu_mode;
    if (dfu_mode)
        dfu_state.state = DFU_IDLE;
    else
        dfu_state.state = APP_IDLE;
}

void DFU_Setup(struct _USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    uint8_t requestType = req->bmRequest & USB_REQ_TYPE_MASK;
    switch (requestType)
    {
    case USB_REQ_TYPE_CLASS:
        if (USBD_CAMERA_handle.dfu_mode)
            DFU_SetupClass_DFU(pdev, req);
        else
            DFU_SetupClass_APP(pdev, req);
        break;
    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
        case USB_REQ_GET_STATUS:
            DFU_GetStatus(pdev, req);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            DFU_GetDescriptor(pdev, req);
            break;
        case USB_REQ_GET_INTERFACE:
            DFU_GetInterface(pdev, req);
            break;
        case USB_REQ_SET_INTERFACE:
            DFU_SetInterface(pdev, req);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}
