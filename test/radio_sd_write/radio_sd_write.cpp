/**
 * @file sd_write.cpp
 * @author csg83
 *
 * @brief Unit test of SD writes over SDIO or SPI
 */

#include "pico/stdlib.h"
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"
#include "tusb.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "telemetry.hpp"

#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>

#define LED 25

#define RFM_TX 4
#define RFM_RX 5
#define UART_PORT uart1

// Timeout Constants
#define RESPONSE_TIMEOUT_MS 1000 // Time to wait after last received character
#define MAX_RESPONSE_SIZE 1024   // Maximum expected response size

FIL fil;
const char *command_enter_seq = "+++";
const char *save_values_seq = "AT&W\r";
const char *reboot_seq = "ATZ\r";
const int rfm_baudrate = 115200;

// Time to wait between reads
const int WAIT_TIME_MS = 2000;

void print_buffer_hex(const char *buffer, size_t size)
{
    printf("Buffer bytes (hex): ");
    for (size_t j = 0; j < size; j++)
    {
        printf("%02X ", (unsigned char)buffer[j]);
    }
    printf("\n");
}

void print_command(const char *str)
{
    printf("Sending Command: ");

    // Keep going until null terminator is reached
    while (*str)
    {
        if (*str != '\r')
        {
            putchar(*str);
        }
        str++;
    }

    printf("\n");
}

void init_uart()
{
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

// Function to send a command and wait for its complete response
bool send_command_and_wait_response(const char *cmd, char *response_buffer, size_t buffer_size, bool write_to_sd)
{

    // Clear the response buffer
    // TODO: Experiment if printf() a buffer is significantly
    // faster than printf() a char as it comes in
    memset(response_buffer, 0, buffer_size);
    size_t response_index = 0;

    // Send the command via UART
    print_command(cmd); // Log the command being sent
    uart_write_blocking(UART_PORT, (const uint8_t *)cmd, strlen(cmd));

    if (cmd == command_enter_seq)
    {
        printf("Command mode starting...\n");
        sleep_ms(2000);
    }

    if (uart_is_readable_within_us(UART_PORT, 1000000))
    {
        while (uart_is_readable_within_us(UART_PORT, 1000000))
        {
            // while (uart_is_readable(UART_PORT)) {
            char c = uart_getc(UART_PORT);

            // Append the character to the response buffer if there's space
            if (response_index < (buffer_size - 1))
            {
                response_buffer[response_index++] = c;
                response_buffer[response_index] = '\0'; // Null-terminate
            }
            else
            {
                printf("Response buffer overflow.\n");
                return false;
            }
            printf("%c", c);
        }
    }

    if (write_to_sd)
    {
        f_printf(&fil, "%s", response_buffer);
    }

    printf("\nComplete Response Received.\n\n");

    return true;
}

void get_all_results(bool write_to_sd = false)
{
    // Buffer to store responses
    char response[MAX_RESPONSE_SIZE];
    send_command_and_wait_response(command_enter_seq, response, sizeof(response), write_to_sd);

    std::vector<const char *> commands = {
        "ATI\r",
        "ATI2\r",
        "ATI3\r",
        "ATI4\r",
        "ATI5\r",
        "ATI6\r",
        "ATI7\r",
        "ATI8\r",
        "ATI9\r"};
    for (const char *cmd : commands)
    {
        send_command_and_wait_response(cmd, response, sizeof(response), write_to_sd);
    }
}

void receive_telemetry()
{

    const size_t TELEM_SIZE_BYTES = sizeof(Telemetry) + 1;
    while (false)
    {
        // TODO: Idea - Use a circular buffer to read data
        uint8_t packet[TELEM_SIZE_BYTES];
        uart_read_blocking(UART_PORT, packet, TELEM_SIZE_BYTES);
        if (packet[TELEM_SIZE_BYTES - 1] != 0xE2)
        {
            printf("Packet not found. Here's what's in the buffer: \n");
            print_buffer_hex((const char *)packet, TELEM_SIZE_BYTES);
            printf("\n\n");
            continue;
        }
        Telemetry *data = reinterpret_cast<Telemetry *>(packet);

        printf("Packet found. Contents of buffer: \n");
        print_buffer_hex((const char *)packet, TELEM_SIZE_BYTES);
        printf("\n\n");

        sleep_ms(WAIT_TIME_MS);
    }

    while (true)
    {
        // Synchronize: read bytes until we find a valid UBX header.
        int sync_attempts = 0;
        int MAX_SYNC_ATTEMPTS = 100;
        while (sync_attempts < MAX_SYNC_ATTEMPTS)
        {
            uint8_t byte;
            // Read a single byte
            uart_read_blocking(UART_PORT, &byte, 1);

            if (byte == 0xE2)
            { // potential start of header
                uint8_t packet[TELEM_SIZE_BYTES];

                uart_read_blocking(UART_PORT, packet, TELEM_SIZE_BYTES - 1);

                printf("Packet found. Contents of buffer: \n");
                print_buffer_hex((const char *)packet, TELEM_SIZE_BYTES);
                printf("\n\n");
                break;
            }
            sync_attempts++;
        }

        sleep_ms(WAIT_TIME_MS);
    }
}

bool receive_string(const uint32_t timeout_s, const bool write_to_sd)
{

    char response_buffer[MAX_RESPONSE_SIZE];
    size_t response_index = 0;

    // Get the current time as the start time (microseconds)
    const absolute_time_t start_time = get_absolute_time();

    while (true)
    {
        // If there isn't data after 1ms, the data burst should be over
        // while (uart_is_readable_within_us(UART_PORT, 100000000)) {

        while (uart_is_readable_within_us(UART_PORT, 10000))
        {

            const char c = uart_getc(UART_PORT);
            printf("%c", c);
            // const bool end_of_message = (c == '\0');
            // if (end_of_message)
            // {
            //     break;
            // }

            // Append the character to the response buffer if there's space
            if (response_index < (MAX_RESPONSE_SIZE - 1))
            {
                response_buffer[response_index++] = c;
                response_buffer[response_index] = '\0'; // Null-terminate
            }
            else
            {

                // if (write_to_sd) {
                //     f_printf(&fil, "%s", response_buffer);
                // }
                // printf("\"(%s)\"", response_buffer);

                // memset(response_buffer, 0, MAX_RESPONSE_SIZE);
                // response_index = 0;

                printf("Response buffer overflow: \n");
                for (int i = 0; i < response_index; i++)
                {
                    printf("%c", response_buffer[i]);
                }
                return false;
            }
            // printf("%c, ", c);
        }

        // while (uart_is_readable_within_us(UART_PORT, 100000000)) {

        //     char c = uart_getc(UART_PORT);
        //     if (c == '\0') {
        //         continue;
        //     }
        //     printf("%c", c);
        // }

        // If data was written
        const bool data_received = (response_index != 0);
        if (data_received)
        {
            if (write_to_sd)
            {
                f_printf(&fil, "%s", response_buffer);
            }
            printf("\"%s\"\n", response_buffer);

            memset(response_buffer, 0, MAX_RESPONSE_SIZE);
            response_index = 0;

            printf("\n\n\n\n\n");
        }

        // Calculate elapsed time in seconds
        const uint64_t elapsed_s = absolute_time_diff_us(start_time, get_absolute_time()) / 1000000;

        // Check if the elapsed time has exceeded the timeout
        const bool timeout_exceeded = elapsed_s >= timeout_s;
        if (timeout_exceeded)
        {
            printf("Telemetry reception timed out after %u s.\n", timeout_s);
            break;
        }
    }

    return true;
}

// This function is for receving radio telemetry, without writing SD card
int main()
{
    const bool WRITE_TO_SD = false;

    stdio_init_all();

    init_uart();

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);

    while (!tud_cdc_connected())
    {
        sleep_ms(500);
    }
    printf("Starting\n");

    // Read radio data
    uint32_t telemtry_timeout_s = 500000;
    receive_string(telemtry_timeout_s, WRITE_TO_SD);
    // receive_telemetry();
    // get_all_results(WRITE_TO_SD);

    printf("Data written\n");

    sleep_ms(500);
}

int t_main()
{
    bool write_to_sd = true;

    stdio_init_all();

    init_uart();

    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
    gpio_put(LED, 1);

    printf("Starting\n");
    while (!tud_cdc_connected())
    {
        sleep_ms(500);
    }
    printf("Connected\n");

    FATFS fs;

    FRESULT fr = f_mount(&fs, "", 1);
    if (FR_OK != fr)
    {
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("Card mounted\n");

    const char *const filename = "1-17-test2.txt";
    fr = f_open(&fil, filename, FA_OPEN_APPEND | FA_WRITE);

    if (FR_OK != fr && FR_EXIST != fr)
    {
        panic("f_open(%s) error: %s (%d)\n", filename, FRESULT_str(fr), fr);
    }
    printf("File opened\n");

    // Read radio data
    uint32_t telemtry_timeout_s = 30;
    receive_string(telemtry_timeout_s, write_to_sd);
    // get_all_results();
    printf("Data written\n");

    // if (f_printf(&fil, "Hello, world!\n") < 0) {
    //     printf("f_printf failed\n");
    // }

    fr = f_close(&fil);
    if (FR_OK != fr)
    {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    printf("File closed\n");

    f_unmount("");
    printf("Card unmounted\n");

    return 0;
}