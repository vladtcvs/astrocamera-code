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
    unsigned gain;
    uint32_t exposure;

    int target_temperature;
    enum exposure_mode_e exposure_mode;
    enum exposure_state_e state;
    bool tec;
    bool fan;
    int window_heater;
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

static uint8_t serial_data_cb(const uint8_t *data, size_t len)
{
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


void core_init(struct usb_context_s *ctx)
{
    usb_ctx = ctx;
    usb_ctx->exposure = exposure_cb;
    usb_ctx->exposure_mode = exposure_mode_cb;
    usb_ctx->set_power_settings = set_power_settings_cb;
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

void core_serial_send_function(void *arg)
{
    const char data[] = "test\n";
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
    while (1) {
        while (send_serial_data((const uint8_t*)data, strlen(data)) == USBD_BUSY)
            vTaskDelay(1);

        vTaskDelay(xDelay);
    }
}

static void start_exposure(void)
{
    while (send_shutter(true) == USBD_BUSY)
        vTaskDelay(1);
    state.state = EXPOSURING;
}

static void complete_exposure(void)
{
    while (send_shutter(false) == USBD_BUSY)
        vTaskDelay(1);
    state.state = READING;
    read_ccd();
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
    state.state = IDLE;
}

void core_process_exposure_cb(unsigned exposure)
{
    switch (state.exposure_mode) {
    case FREERUN:
        // Do nothing
        break;
    case TRIGGERED_BEGIN:
        if (state.state == IDLE) {
            if (exposure > 0) {
                start_exposure_timer(exposure);
                start_exposure();
            } else {
                // ignore
            }
        } else if (state.state == EXPOSURING) {
            if (exposure == 0) {
                complete_exposure();
            } else {
                // error
            }
        } else {
            // error
        }
        break;
    case TRIGGERED_BEGIN_END:
        if (state.state == IDLE) {
            if (exposure > 0) {
                start_exposure();
            } else {
                // ignore
            }
        } else if (state.state == EXPOSURING) {
            if (exposure == 0) {
                complete_exposure();
            } else {
                // error
            }
        } else {
            // error
        }
        break;
    default:
        // error
        break;
    }
}

void core_process_exposure_mode_cb(unsigned mode)
{
    switch (mode) {
    case 0:
        state.exposure_mode = FREERUN;
        break;
    case 1:
        state.exposure_mode = TRIGGERED_BEGIN;
        break;
    case 2:
        state.exposure_mode = TRIGGERED_BEGIN_END;
        break;
    default:
        // error
        break;
    }
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
