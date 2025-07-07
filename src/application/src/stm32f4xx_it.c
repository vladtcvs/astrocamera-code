#include "system_config.h"
#include "stm32f4xx.h"

#include <FreeRTOS.h>
#include <task.h>
#include <stdbool.h>

#include "stm32f4xx_hal_pcd.h"

#define LOOP() {__disable_irq(); while(1);}

void NMI_Handler(void)
{
    LOOP();
}

void HardFault_Handler(void)
{
    LOOP();
}

void MemManage_Handler(void)
{
    LOOP();
}

void BusFault_Handler(void)
{
    LOOP();
}

void UsageFault_Handler(void)
{
    LOOP();
}


void DebugMon_Handler(void)
{
    // do nothing
}

/*
  Handlers SVC_Handler and PendSV_Handler are implemented by FreeRTOS
*/

void xPortSysTickHandler( void );
bool freertos_tick = false;
void SysTick_Handler(void)
{
    HAL_IncTick();
    if (freertos_tick)
        xPortSysTickHandler();
}

extern PCD_HandleTypeDef hpcd_USB_OTG_HS;
void OTG_HS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_HS);
}


