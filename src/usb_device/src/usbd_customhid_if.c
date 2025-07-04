
#include <stdbool.h>
#include "usbd_customhid_if.h"

void core_process_exposure_cb(int exposure);
void core_process_exposure_mode_cb(int mode);
void core_process_target_temperature_cb(int target_temperature);
void core_process_window_heater_cb(int window_heater);
void core_process_fan_cb(bool fan);
void core_process_tec_cb(bool tec);

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

        // Input 

        0x85, 0x01, //   Report ID (1)
        // ----- Current Temperature (16-bit) -----
        0x09, 0x10,       //   Usage (Current Temperature)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,       //   Report Size (16 bits)
        0x95, 0x01,       //   Report Count (1)
        0x81, 0x02,       //   Input (Data, Var, Abs)

        0x85, 0x02, //   Report ID (2)
        // ----- TEC status (1 bit) -
        0x09, 0x11,       //   Usage (TEC status)
        0x15, 0x00,       //   Logical Min (0)
        0x25, 0x01,       //   Logical Max (1)
        0x75, 0x01,       //   Report Size (1 bits)
        0x95, 0x01,       //   Report Count (1)
        0x81, 0x02,       //   Input (Data, Var, Abs)

        // ----- FAN status (1 bit) -
        0x09, 0x12,       //   Usage (FAN status)
        0x15, 0x00,       //   Logical Min (0)
        0x25, 0x01,       //   Logical Max (1)
        0x75, 0x01,       //   Report Size (1 bits)
        0x95, 0x01,       //   Report Count (1)
        0x81, 0x02,       //   Input (Data, Var, Abs)

        // ----- HEATER status (4 bit) -
        0x09, 0x13,       //   Usage (FAN status)
        0x15, 0x00,       //   Logical Min (0)
        0x25, 0x10,       //   Logical Max (16)
        0x75, 0x04,       //   Report Size (4 bits)
        0x95, 0x01,       //   Report Count (1)
        0x81, 0x02,       //   Input (Data, Var, Abs) — Read Only on host

        // Padding to next byte
        0x75, 0x02,       // Report Size = 2
        0x95, 0x01,       // Report Count = 1
        0x81, 0x03,       // Input (Constant)

        0x85, 0x03, //   Report ID (3)
        // ----- exposure status (1 bit) -
        0x09, 0x14,       //   Usage (exposure status)
        0x15, 0x00,       //   Logical Min (0)
        0x25, 0x01,       //   Logical Max (1)
        0x75, 0x01,       //   Report Size (1 bits)
        0x95, 0x01,       //   Report Count (1)
        0x81, 0x02,       //   Input (Data, Var, Abs)

        // Padding to next byte
        0x75, 0x07,       // Report Size = 7
        0x95, 0x01,       // Report Count = 1
        0x81, 0x03,       // Input (Constant)

        // Output 

        0x85, 0x01, //   Report ID (1)

        // ----- Target Temperature (RW, 16-bit) -----
        0x09, 0x10,       //   Usage (Target Temperature)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,       //   Report size (16 bits)
        0x95, 0x01,
        0x91, 0x02,       //   Output

        0x85, 0x02, //   Report ID (2)

        // ----- TEC Enabled (1-bit) -----
        0x09, 0x11,    // Usage (TEC Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,    // Report Size (1 bit)
        0x95, 0x01,    // Report Count (1)
        0x91, 0x02,    // Output

        // ----- Fan Enabled (1-bit) -----
        0x09, 0x12,    //   Usage (Fan Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x01,    // Logical Maximum (1)
        0x75, 0x01,    // Report Size (1 bit)
        0x95, 0x01,    // Report Count (1)
        0x91, 0x02,    // Output

        // ----- Heater Power (4-bit) -----
        0x09, 0x13,    //   Usage (Heater Enabled)
        0x15, 0x00,    // Logical Minimum (0)
        0x25, 0x10,    // Logical Maximum (16)
        0x75, 0x04,    // Report Size (4 bit)
        0x95, 0x01,    // Report Count (1)
        0x91, 0x02,    // Output

        // Padding to next byte
        0x75, 0x07,       // Report Size = 2
        0x95, 0x01,       // Report Count = 1
        0x91, 0x03,       // Input (Constant)

        0x85, 0x03, //   Report ID (3)

        // ----- exposure (16 bit) -
        0x09, 0x14,       //   Usage (exposure)
        0x15, 0x00,       //   Logical Min (0)
        0x26, 0xFF, 0x7F, //   Logical Max (32767)
        0x75, 0x10,       //   Report Size (16 bits)
        0x95, 0x01,       //   Report Count (1)
        0x91, 0x02,       //   Input (Data, Var, Abs)

        0x85, 0x04, //   Report ID (4)

        // ----- exposure mode (2 bit)
        0x09, 0x15,       //   Usage (exposure status)
        0x15, 0x00,       //   Logical Min (0)
        0x25, 0x02,       //   Logical Max (2)
        0x75, 0x02,       //   Report Size (2 bits)
        0x95, 0x01,       //   Report Count (1)
        0x91, 0x02,       //   Output (Data, Var, Abs)

        // Padding to next byte
        0x75, 0x06,       // Report Size = 6
        0x95, 0x01,       // Report Count = 1
        0x91, 0x03,       // Output (Constant)

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

    switch (report_id) {
        case 1: {
            uint16_t data_l = report_buffer[1];
            uint16_t data_h = report_buffer[2];
            core_process_target_temperature_cb(data_h << 8 | data_l);
            break;
        }
        case 2: {
            uint8_t data = report_buffer[1];
            bool tec = data & 0x01;
            bool fan = (data >> 1) & 0x01;
            int heater = (data >> 2) & 0x0F;
            core_process_tec_cb(tec);
            core_process_fan_cb(fan);
            core_process_window_heater_cb(heater);
            break;
        }
        case 3: {
            uint16_t data_l = report_buffer[1];
            uint16_t data_h = report_buffer[2];
            core_process_exposure_cb(data_h << 8 | data_l);
            break;
        }
        case 4: {
            uint8_t data = report_buffer[1];
            int exposure_mode = data & 0x03;
            core_process_exposure_mode_cb(exposure_mode);
            break;
        }
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
