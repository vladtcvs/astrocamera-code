#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>

#include "usb_device.h"
#include "shell.h"

#define CMDLINE_LEN 32

void process_command(const char *cmd)
{
    printf("Received command: %s\n", cmd);
}

void shell_task_function(void *arg)
{
    char cmdline[CMDLINE_LEN];
    size_t len = 0;
    puts("> ");
    while (1) {
        int c = getchar();
        if (c < 0)
            continue;
        if (c == '\n') {
            puts("\r\n");
            process_command(cmdline);
            puts("> ");
            cmdline[0] = 0;
            len = 0;
        } else if (c == '\r') {
            // ignore
        } else {
            if (len < CMDLINE_LEN-1) {
                cmdline[len++] = c;
                cmdline[len] = 0;
                putchar(c);
            }
        }
    }
}
