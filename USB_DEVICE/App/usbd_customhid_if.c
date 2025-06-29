
#include "usbd_customhid_if.h"

static int8_t TEMPLATE_CUSTOM_HID_Init(void);
static int8_t TEMPLATE_CUSTOM_HID_DeInit(void);
static int8_t TEMPLATE_CUSTOM_HID_OutEvent(uint8_t event_idx, uint8_t state);
static uint8_t *TEMPLATE_CUSTOM_HID_GetReport(uint16_t *ReportLength);

/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceHS;

__ALIGN_BEGIN static uint8_t TEMPLATE_CUSTOM_HID_ReportDesc[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END =
    {
        0x06, 0x00, 0xFF, // Usage Page (Vendor-defined 0xFF00)
        0x09, 0x01,       // Usage (Vendor-defined)
        0xA1, 0x01,       // Collection (Application)

        0x85, 0x01, //   Report ID (1)

        // ----- Current Temperature (RO, 16-bit) -----
        0x09, 0x10,       //   Usage (Current Temperature)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,       //   Report Size (16 bits)
        0x95, 0x01,       //   Report Count (1)
        0xB1, 0x02,       //   Feature (Data, Var, Abs) — Read Only on host

        // ----- Target Temperature (RW, 16-bit) -----
        0x09, 0x11, //   Usage (Target Temperature)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,
        0x95, 0x01,
        0xB1, 0x02, //   Feature (R/W)

        // ----- TEC Enabled (RW, 1-bit) -----
        0x09, 0x12,    // Usage (TEC Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,    // Report Size (1 bit)
        0x95, 0x01,    // Report Count (1)
        0xB1, 0x02,    // Feature (Data, Variable, Absolute) — R/W

        // ----- Fan Enabled (RW, 1-bit) -----
        0x09, 0x13, //   Usage (Fan Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,
        0x95, 0x01,
        0xB1, 0x02,

        // ----- Heater Enabled (RW, 1-bit) -----
        0x09, 0x14, //   Usage (Heater Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,
        0x95, 0x01,
        0xB1, 0x02,

        // ----- Padding to align to next byte -----
        0x75, 0x05,
        0x95, 0x01,
        0xB1, 0x03, //   Feature (Const, Var, Abs) — padding

        0xC0 // End Collection
};

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops =
    {
        TEMPLATE_CUSTOM_HID_ReportDesc,
#ifdef USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED
        USBD_CUSTOM_HID_REPORT_DESC_SIZE,
#endif /* USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED */
        TEMPLATE_CUSTOM_HID_Init,
        TEMPLATE_CUSTOM_HID_DeInit,
        TEMPLATE_CUSTOM_HID_OutEvent,
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
        TEMPLATE_CUSTOM_HID_CtrlReqComplete,
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
        TEMPLATE_CUSTOM_HID_GetReport,
#endif /* USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED */
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  TEMPLATE_CUSTOM_HID_Init
 *         Initializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_CUSTOM_HID_Init(void)
{
    return (0);
}

/**
 * @brief  TEMPLATE_CUSTOM_HID_DeInit
 *         DeInitializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t TEMPLATE_CUSTOM_HID_DeInit(void)
{
    /*
       Add your deinitialization code here
    */
    return (0);
}

/**
 * @brief  TEMPLATE_CUSTOM_HID_Control
 *         Manage the CUSTOM HID class events
 * @param  event_idx: event index
 * @param  state: event state
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */

static int8_t TEMPLATE_CUSTOM_HID_OutEvent(uint8_t event_idx, uint8_t state)
{
    UNUSED(event_idx);
    UNUSED(state);

    /* Start next USB packet transfer once data processing is completed */
    if (USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceHS) != (uint8_t)USBD_OK)
    {
        return -1;
    }

    return (0);
}

/**
 * @brief  TEMPLATE_CUSTOM_HID_GetReport
 *         Manage the CUSTOM HID control Get Report request
 * @param  event_idx: event index
 * @param  state: event state
 * @retval return pointer to HID report
 */
static uint8_t *TEMPLATE_CUSTOM_HID_GetReport(uint16_t *ReportLength)
{
    UNUSED(ReportLength);
    uint8_t *pbuff = NULL;

    return (pbuff);
}
