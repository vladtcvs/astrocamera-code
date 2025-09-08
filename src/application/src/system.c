#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>
#include <stm32f4xx_hal.h>
#include "usb_device.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while (1)
    {
    }
}

void vApplicationTickHook(void)
{
}

void vApplicationIdleHook(void)
{
}


void NMI_Handler(void)
{
    while (1)
        ;
}

void MemManage_Handler(void)
{
    while (1)
        ;
}

void BusFault_Handler(void)
{
    while (1)
        ;
}

void UsageFault_Handler(void)
{
    while (1)
        ;
}

/* Defined by FreeRTOS
void SVC_Handler(void)
{
}
*/

void DebugMon_Handler(void)
{
}


/* Defined by FreeRTOS
void PendSV_Handler(void)
{
}
*/

void xPortSysTickHandler(void);
bool freertos_tick = false;
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (freertos_tick)
        xPortSysTickHandler();
}

int _write(int file, char *ptr, int len)
{
    if (file != 1 && file != 2)
        return 0;

    while (len > 0) {
        int blen = len;
        if (blen > CAMERA_CDC_DATA_EPIN_SIZE)
            blen = CAMERA_CDC_DATA_EPIN_SIZE;
        while (send_serial_data(ptr, blen) == USBD_BUSY)
            ;
        ptr += blen;
        len -= blen;
    }
    return len;
}

#define RXBUFLEN 64
static char rxbuf[RXBUFLEN];
static uint16_t head = 0, tail = 0;

void system_save_rx_data(const uint8_t *data, size_t len)
{
    while (len > 0) {
        rxbuf[tail] = *(data++);
        tail = (tail + 1) % RXBUFLEN;
    }
}

int _read(int file, char *ptr, int len)
{
    if (len > rxbuflen)
        len = rxbuflen;
    if (len == 0)
        return 0;

    memcpy(ptr, rxbuf, len);
    memmove(rxbuf, rxbuf + len, rxbuflen - len);
    rxbuflen -= len;
    return len;
}

void _close(int file)
{

}

off_t _lseek(int file, off_t offset, int whence)
{
    return 0;
}

int _isatty_r (struct _reent *, int)
{
    return 0;
}

int _fstat(int file, struct stat *st)
{
    return 0;
}
