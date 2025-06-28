
#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_hid.h"

/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceHS;

/**
 * Init USB device Library, add supported class and start the library
 * @retval None
 */
void MX_USB_DEVICE_Init(void)
{
    /* Reset PHY */
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(USB_RST_GPIO_Port, USB_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);

    /* Init Device Library, add supported class and start the library. */
    if (USBD_Init(&hUsbDeviceHS, &HS_Desc, DEVICE_HS) != USBD_OK)
    {
        Error_Handler();
    }
    if (USBD_RegisterClass(&hUsbDeviceHS, &USBD_HID) != USBD_OK)
    {
        Error_Handler();
    }
    if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
    {
        Error_Handler();
    }
}
