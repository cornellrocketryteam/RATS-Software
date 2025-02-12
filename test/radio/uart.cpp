/**
 * @file main.cpp
 * @author YOUR NETID HERE
 * 
 * @brief The entry point for the program
 */

#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/uart.h"

#define LED 25

#define RFM_TX 0
#define RFM_RX 1
#define UART_PORT uart0

const char *command_enter_seq = "+++";
const char *save_values_seq = "AT&W\r";
const char *reboot_seq = "ATZ\r";
const int rfm_baudrate = 115200;

/* UART pins are initialized by:
    1) Initializating UART pins with desired baudrate
    2) Setting what the funtion does
    3) Setting the UART format, which in order is:
        - Data Size
        - # of end bits
        - Whether parity bit will exist or not
    4) Setting whether UART will have TX/RX flow control
    5) Setting if FIFO will be enabled */
void init_uart() {
    // By default, the radio module has baudrate value rfm_baudrate, so we
    // set that value on the local UART port
    uart_init(UART_PORT, rfm_baudrate);

    
    gpio_set_function(RFM_TX, GPIO_FUNC_UART);
    gpio_set_function(RFM_RX, GPIO_FUNC_UART);

    // Set UART format to 8 data bits, 1 stop bit, no parity
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);

    // Set flow control to false for both
    uart_set_hw_flow(UART_PORT, false, false);

    uart_set_fifo_enabled(UART_PORT, true);
}



int main() {
    stdio_init_all();

    init_uart();

    gpio_init(LED);

    gpio_set_dir(LED, GPIO_OUT);

    sleep_ms(5000);
    int i = 0;
    while (true) {

        if (i == 0) {
            uart_write_blocking(UART_PORT, (const uint8_t *)command_enter_seq, strlen(command_enter_seq));
            printf("Wrote \"%s\"\n", command_enter_seq);
        }
        else if (i == 1) {
            char command[20];
            sprintf(command, "ATI\r");
            uart_write_blocking(UART_PORT, (const uint8_t *)command, strlen(command));
            printf("Wrote \"%s\"\n", command);
        }

        if (i < 2) {
            i++;
        }
        else {
            while (true) {
                // Check if data is available
                if (uart_is_readable(UART_PORT)) {
                    char ch = uart_getc(UART_PORT);
                    printf("%c", ch);
                }
            }
        }
        printf("Hello, world!\n");
        gpio_put(LED, 1);
        sleep_ms(1000);
        gpio_put(LED, 0);
        sleep_ms(1000);
    }

    return 0;
}