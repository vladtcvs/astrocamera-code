#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif
    void NMI_Handler(void);
    void HardFault_Handler(void);
    void MemManage_Handler(void);
    void BusFault_Handler(void);
    void UsageFault_Handler(void);
    void SVC_Handler(void);
    void DebugMon_Handler(void);
    void PendSV_Handler(void);
    void SysTick_Handler(void);
    void OTG_HS_IRQHandler(void);

    extern bool freertos_tick;

#ifdef __cplusplus
}
#endif
