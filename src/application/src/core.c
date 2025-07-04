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
    enum exposure_mode_e exposure_mode;
    enum exposure_state_e state;
    bool tec;
    bool fan;
    int window_heater;
};

static struct core_state_s state;


struct usb_context_s;

void sensors_poll_function(void *ctx)
{
    int current_temperature = 2961; // 296.1 K = 23 C
    const TickType_t xDelay = 100 / portTICK_PERIOD_MS;
    while (1) {
        send_sensors((struct usb_context_s *)ctx, current_temperature);
        send_status((struct usb_context_s *)ctx, state.tec, state.fan, state.window_heater);
        vTaskDelay(xDelay);
    }
}

void exposure_start(struct usb_context_s *ctx)
{
    send_shutter(ctx, true);
}

void exposure_finish(struct usb_context_s *ctx)
{
    send_shutter(ctx, false);
}

void process_exposure_cb(bool exposure)
{

}

void process_exposure_mode_cb(int mode)
{

}

void process_target_temperature_cb(int target_temperature)
{

}

void process_window_heater_cb(int window_heater)
{

}

void process_fan_cb(bool fan)
{

}

void process_tec_cb(bool tec)
{

}
