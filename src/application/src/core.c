#include <stdbool.h>
#include <FreeRTOS.h>
#include <task.h>
#include <usb_device.h>

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
    int target_temperature;
    enum exposure_mode_e exposure_mode;
    enum exposure_state_e state;
    bool tec;
    bool fan;
    int window_heater;
};

static struct core_state_s state;


struct usb_context_s;
struct usb_context_s *usb_ctx;

void core_set_usbctx(struct usb_context_s *ctx)
{
    usb_ctx = ctx;
}

void core_sensors_poll_function(void *arg)
{
    int current_temperature = 2961; // 296.1 K = 23 C
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
    while (1) {
        send_sensors(usb_ctx, current_temperature);
        send_status(usb_ctx, state.tec, state.fan, state.window_heater);
        vTaskDelay(xDelay);
    }
}

static void start_exposure(void)
{
    send_shutter(usb_ctx, true);
}

static void complete_exposure(void)
{
    send_shutter(usb_ctx, false);
}

static void start_exposure_timer(int exposure)
{

}

void core_process_exposure_cb(int exposure)
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

void core_process_exposure_mode_cb(int mode)
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

void core_process_target_temperature_cb(int target_temperature)
{
    state.target_temperature = target_temperature;
}

void core_process_window_heater_cb(int window_heater)
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
