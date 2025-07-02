
#include <stdbool.h>
#include "usbd_customhid_if.h"

void process_trigger(bool trigger);

static int8_t HID_Init(void);
static int8_t HID_DeInit(void);
static int8_t HID_OutEvent(uint8_t *report_buffer);
static int8_t HID_CtrlReqComplete(uint8_t request, uint16_t wLength);
static uint8_t *HID_GetReport(uint16_t *ReportLength);

/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceHS;

__ALIGN_BEGIN static uint8_t HID_ReportDesc[] __ALIGN_END =
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
        0x81, 0x02,       //   Input (Data, Var, Abs) — Read Only on host

        0x85, 0x01, //   Report ID (1)

        // ----- Target Temperature (RW, 16-bit) -----
        0x09, 0x11, //   Usage (Target Temperature)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,
        0x95, 0x01,
        0xB1, 0x02,       //   Feature (R/W)

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
        0xB1, 0x02,    // Feature

        // ----- Heater Enabled (RW, 1-bit) -----
        0x09, 0x14, //   Usage (Heater Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,
        0x95, 0x01,
        0xB1, 0x02,    // Feature

        // ----- Mode (RW, 2-bit) -----
        0x09, 0x15, //   Usage (Mode)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x02,    // Logical Maximum (2)
        0x75, 0x02,
        0x95, 0x01,
        0xB1, 0x02,    // Feature

        0x85, 0x01, //   Report ID (1)

        // ----- Exposure trigger (WO, 1-bit) -----
        0x09, 0x20,       // Usage (Vendor-defined or your assigned usage for Trigger)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x01,       // Logical Maximum (1)
        0x75, 0x01,       // Report Size = 1 bit
        0x95, 0x01,       // Report Count = 1 (1 bit)
        0x91, 0x02,       // Output (Data, Variable, Absolute)

        // ----- Padding of FEATURE to align to next byte -----
        0x75, 0x04,
        0x95, 0x01,
        0xB1, 0x03, //   Feature (Const, Var, Abs) — padding

        // ----- Padding of INPUT to align to next byte -----
        
        // ----- Padding of OUTPUT to align to next byte -----
        0x75, 0x07,
        0x95, 0x01,
        0x91, 0x03, //   Output (Const, Var, Abs) — padding

        0xC0 // End Collection
};

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops =
    {
        .pReport = HID_ReportDesc,
#ifdef USBD_CUSTOMHID_REPORT_DESC_SIZE_ENABLED
        .wReportDescLen = USBD_CUSTOM_HID_REPORT_DESC_SIZE,
#endif
        .Init = HID_Init,
        .DeInit = HID_DeInit,
        .OutEvent = HID_OutEvent,
#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
        .CtrlReqComplete = HID_CtrlReqComplete,
#endif

#ifdef USBD_CUSTOMHID_CTRL_REQ_GET_REPORT_ENABLED
        .GetReport = HID_GetReport,
#endif
};

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  HID_Init
 *         Initializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t HID_Init(void)
{
    return (0);
}

/**
 * @brief  HID_DeInit
 *         DeInitializes the CUSTOM HID media low layer
 * @param  None
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t HID_DeInit(void)
{
    /*
       Add your deinitialization code here
    */
    return (0);
}

/**
 * @brief  HID_Control
 *         Manage the CUSTOM HID class events
 * @param  report_buffer: report buffer
 * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t HID_OutEvent(uint8_t *report_buffer)
{
    uint8_t report_id = report_buffer[0];

    if (report_id == 1) {
        uint8_t data = report_buffer[1];
        bool trigger = data & 0x01;
        process_trigger(trigger);
    }

    /* Start next USB packet transfer once data processing is completed */
    if (USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceHS) != (uint8_t)USBD_OK)
    {
        return -1;
    }

    return (0);
}

/**
 * @brief  HID_GetReport
 *         Manage the CUSTOM HID control Get Report request
 * @param  event_idx: event index
 * @param  state: event state
 * @retval return pointer to HID report
 */
static uint8_t *HID_GetReport(uint16_t *ReportLength)
{
    uint8_t *pbuff = NULL;

    return (pbuff);
}

#ifdef USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED
/**
  * @brief  HID_CtrlReqComplete
  *         Manage the HID control request complete
  * @param  request: control request
  * @param  wLength: request wLength
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t HID_CtrlReqComplete(uint8_t request, uint16_t wLength)
{
  UNUSED(wLength);

  switch (request)
  {
    case CUSTOM_HID_REQ_SET_REPORT:

      break;

    case CUSTOM_HID_REQ_GET_REPORT:

      break;

    default:
      break;
  }

  return (0);
}
#endif /* USBD_CUSTOMHID_CTRL_REQ_COMPLETE_CALLBACK_ENABLED */
