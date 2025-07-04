#pragma once

#include <stdbool.h>

void sensors_poll_function(void *ctx);
void process_trigger_cb(bool trigger);
