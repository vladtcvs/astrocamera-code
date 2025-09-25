#include <FreeRTOS.h>
#include <stdint.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "hw/quadspi.h"
#include "usb_device.h"
#include "shell.h"

#define CMDLINE_LEN 32

void ctl_spi_begin();
void ctl_spi_finish(void);
uint8_t ctl_spi_transfer(uint8_t data);

void process_command(const char *cmd)
{
    uint32_t addr;
    if (!strncmp(cmd, "help", 4U)) {
        printf("Usage: <cmd> <args>\r\n");
        printf("Commands:\r\n");
        printf("  readctl\r\n");
        printf("  writectl\r\n");
    } else if (!strncmp(cmd, "rc ", 3U)) {
        int addr;
        int num;
        int num_read = sscanf(cmd, "rc %X %i", &addr, &num);
        if (num_read != 2) {
            printf("Usage: rc <hex ADDR> <NUM_BYTES>\r\n");
            return;
        }

        if (addr < 0 || addr > 255) {
            printf("Allowed ctl addr = 0x00....0xFF\r\n");
        } else {
            printf("Read ctl at 0x%02X %d bytes\r\n", addr, num);
            ctl_spi_begin();
            vTaskDelay(10);
            ctl_spi_transfer(0x03U);    // Read
            ctl_spi_transfer(addr);     // Send addr
            ctl_spi_transfer(0x00U);    // dummy
            vTaskDelay(10);
            uint8_t cnt;
            for (cnt = 0; cnt < num; cnt++) {
                uint8_t val = ctl_spi_transfer(0x00U);
                printf("0x%02X : 0x%02X\r\n", addr + cnt, val);
                vTaskDelay(10);
            }

            ctl_spi_finish();
            printf("\r\n");
        }
    } else if (!strncmp(cmd, "wc ", 3U)) {
        int val;
        int num_read = sscanf(cmd, "wc %X %X", &addr, &val);
        if (num_read != 2) {
            printf("Usage: wc <hex ADDR> <hex VALUE>\r\n");
            return;
        }

        if (addr < 0 || addr > 255) {
            printf("Allowed ctl addr = 0x00....0xFF\r\n");
        } else {
            printf("Write ctl at 0x%02X = %02X\r\n", addr, val);
            ctl_spi_begin();
            vTaskDelay(10);
            ctl_spi_transfer(0x02U);    // Write
            ctl_spi_transfer(addr);     // Send addr
            ctl_spi_transfer(val);      // value
            vTaskDelay(10);
            ctl_spi_finish();
            printf("\r\n");
        }
    } else if (!strncmp(cmd, "rm ", 3U)) {
        int num;
        int num_read = sscanf(cmd, "rm %X %i", &addr, &num);
        if (num_read != 2) {
            printf("Usage: rm <hex ADDR> <NUM_BYTES>\r\n");
            return;
        }

        if (addr != addr & 0x00FFFFFFU) {
            printf("Allowed mem addr = 0x00000000....0x00FFFFFF\r\n");
        } else {
            static uint8_t buf[16];

            if (num > sizeof(buf))
                num = sizeof(buf);

            printf("Read mem at 0x%06X %d bytes\r\n", addr, num);
            QUADSPI_Read(addr, buf, num);
            uint8_t cnt;
            for (cnt = 0; cnt < num; cnt++)
                printf("0x%06X : 0x%02X\r\n", addr + cnt, buf[cnt]);

            printf("\r\n");
        }
    } else if (!strncmp(cmd, "wm ", 3U)) {
        uint8_t val;
        int num_read = sscanf(cmd, "wm %X %X", &addr, &val);
        if (num_read != 2) {
            printf("Usage: wm <hex ADDR> <hex VALUE>\r\n");
            return;
        }

        if (addr != addr & 0x00FFFFFFU) {
            printf("Allowed mem addr = 0x00000000....0x00FFFFFF\r\n");
        } else {
            printf("Write mem at 0x%06X = %02X\r\n", addr, val);
            QUADSPI_Write(addr, &val, 1);
            printf("\r\n");
        }
    } else {
        printf("Unknown command \"%s\"\r\n", cmd);
    }   
}

void shell_task_function(void *arg)
{
    static char cmdline[CMDLINE_LEN];
    size_t len = 0;
    printf("> ");
    fflush(stdout);
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    bool prev_crlf = false;
    while (1) {
        int symbol = getchar();
        if (symbol == '\n' || symbol == '\r') {
            if (!prev_crlf) {
                putchar('\r');
                putchar('\n');
                fflush(stdout);
                process_command(cmdline);
                putchar('\r');
                putchar('\n');
                putchar('>');
                putchar(' ');
                fflush(stdout);
            }
            cmdline[0] = 0;
            len = 0;
            prev_crlf = true;
        } else if (isprint(symbol)) {
            if (len < CMDLINE_LEN-1) {
                cmdline[len++] = symbol;
                cmdline[len] = 0;
                putchar(symbol);
                fflush(stdout);
            }
            prev_crlf = false;
        } else if (symbol == '\b' || symbol == 0x7F) {
            if (len > 0) {
                cmdline[len] = 0;
                len--;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else {
            prev_crlf = false;
        }
        //puts("test\r\n");
        //vTaskDelay(xDelay);
    }
}
