#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <usb_device.h>

#include "core.h"

enum exposure_mode_e {
    FREERUN = 0,
    TRIGGERED_BEGIN = 1,
    TRIGGERED_BEGIN_END = 2,
};

enum exposure_state_e {
    IDLE = 0,
    EXPOSURING,
    READING,
    UPLOADING,
};

struct core_state_s
{
    unsigned fan;
    unsigned tec;
    unsigned window_heater;
    unsigned trigger_mode;
    unsigned target_temperature;
    unsigned current_temperature;
    unsigned window_temperature;
    unsigned gain;
    unsigned exposure;
};


static struct core_state_s state;
static struct usb_context_s *usb_ctx;
static StaticTimer_t exposure_timer_buffer;
static TimerHandle_t exposure_timer;

static void exposure_timer_cb( TimerHandle_t xTimer );
static void read_ccd(void);

struct usb_context_s;

static uint8_t set_target_temperature_cb(unsigned temperature)
{
    return USBD_OK;
}

static uint8_t set_power_settings_cb(bool TEC, bool fan, int window_heater)
{
    return USBD_OK;
}

static uint8_t exposure_cb(unsigned exposure)
{
    return USBD_OK;
}

static uint8_t exposure_mode_cb(unsigned exposure_mode)
{
    return USBD_OK;
}

void system_save_rx_data(const uint8_t *data, size_t len);
static uint8_t serial_data_cb(const uint8_t *data, size_t len)
{
    system_save_rx_data(data, len);
    return USBD_OK;
}

static uint8_t get_gain(unsigned *gain)
{
    *gain = state.gain;
    return USBD_OK;
}

static uint8_t set_gain(unsigned gain)
{
    state.gain = gain;
    return USBD_OK;
}

static uint8_t get_exposure(uint32_t *exposure)
{
    *exposure = state.exposure;
    return USBD_OK;
}

static uint8_t set_exposure(uint32_t exposure)
{
    state.exposure = exposure;
    return USBD_OK;
}

static uint8_t get_fan(unsigned *fan)
{
    *fan = state.fan;
    return USBD_OK;
}

static uint8_t set_fan(unsigned fan)
{
    state.fan = fan;
    return USBD_OK;
}

static uint8_t get_tec(unsigned *tec)
{
    *tec = state.tec;
    return USBD_OK;
}

static uint8_t set_tec(unsigned tec)
{
    state.tec = tec;
    return USBD_OK;
}

static uint8_t get_window_heater(unsigned *heater)
{
    *heater = state.window_heater;
    return USBD_OK;
}

static uint8_t set_window_heater(unsigned heater)
{
    state.window_heater = heater;
    return USBD_OK;
}

static uint8_t get_trigger_mode(unsigned *trigger_mode)
{
    *trigger_mode = state.trigger_mode;
    return USBD_OK;
}

static uint8_t set_trigger_mode(unsigned trigger_mode)
{
    state.trigger_mode = trigger_mode;
    return USBD_OK;
}

static uint8_t get_target_temperature(unsigned *temperature)
{
    *temperature = state.target_temperature;
    return USBD_OK;
}

static uint8_t set_target_temperature(unsigned temperature)
{
    state.target_temperature = temperature;
    return USBD_OK;
}

static uint8_t get_current_temperature(unsigned *temperature)
{
    *temperature = state.current_temperature;
    return USBD_OK;
}

static uint8_t get_window_temperature(unsigned *temperature)
{
    *temperature = state.window_temperature;
    return USBD_OK;
}

void core_init(struct usb_context_s *ctx)
{
    usb_ctx = ctx;
    usb_ctx->set_target_temperature = set_target_temperature_cb;
    usb_ctx->serial_data = serial_data_cb;
    usb_ctx->get_gain = get_gain;
    usb_ctx->set_gain = set_gain;
    usb_ctx->get_exposure = get_exposure;
    usb_ctx->set_exposure = set_exposure;

    state.exposure = VC_DEFAULT_EXPOSURE;

    exposure_timer = xTimerCreateStatic(
        "ExposureTimer",              // Name
        pdMS_TO_TICKS(1000),          // Period: 1 second
        pdFALSE,                      // Auto reload = false
        NULL,                         // Timer ID
        exposure_timer_cb,            // Callback function
        &exposure_timer_buffer        // Static buffer
    );
}

void core_sensors_poll_function(void *arg)
{
    int current_temperature = 2961; // 296.1 K = 23 C
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    while (1) {
        // TODO: implement
        vTaskDelay(xDelay);
    }
}

static void start_exposure(void)
{
}

static void complete_exposure(void)
{
}

static void read_ccd(void)
{
    core_read_ccd_completed_cb();
}

static void exposure_timer_cb(TimerHandle_t xTimer)
{
    complete_exposure();
}

static void start_exposure_timer(int exposure)
{
    xTimerChangePeriod(exposure_timer, pdMS_TO_TICKS(exposure), 0);
    xTimerStart(exposure_timer, 0);
}

void core_read_ccd_completed_cb(void)
{
}

void core_process_exposure_cb(unsigned exposure)
{   
}

void core_process_exposure_mode_cb(unsigned mode)
{
}

void core_process_target_temperature_cb(unsigned target_temperature)
{
    state.target_temperature = target_temperature;
}

void core_process_window_heater_cb(unsigned window_heater)
{
    state.window_heater = window_heater;
}

void core_process_fan_cb(bool fan)
{
    state.fan = fan;
}

void core_process_tec_cb(bool tec)
{
    state.tec = tec;
}
