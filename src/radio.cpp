/**
 * @file radio.cpp
 * @author sj728
 *
 * @brief Radio module implementation for the RFD900x
 */

#include "radio.hpp"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "telemetry.hpp"
#include "time.h"
#include "pico/time.h"
#include "pins.hpp"
#include <cstdio>
#include <sstream>
#include <cstring>
#include <inttypes.h> // For PRId64

#ifdef RATS_VERBOSE
#define debug_log(...)       \
    do                       \
    {                        \
        printf(__VA_ARGS__); \
    } while (0)
#else
#define debug_log(...) \
    do                 \
    {                  \
    } while (0)
#endif

const uint32_t SYNC_WORD = 0x43525421;

const size_t PACKET_SIZE = sizeof(Telemetry) + sizeof(SYNC_WORD);

void print_buffer_hex(const char *buffer, size_t size)
{
    printf("Buffer bytes (hex): ");
    for (size_t j = 0; j < size; j++)
    {
        printf("%02X ", (unsigned char)buffer[j]);
    }
    printf("\n");
}
bool read_sync_word(void *buffer, int start)
{
    uint32_t sync_word;
    std::memcpy(&sync_word, static_cast<char *>(buffer) + start, sizeof(sync_word));
    // printf("Sync word: %08X\n", sync_word);
    return sync_word == SYNC_WORD;
}

bool Radio::start()
{
    // By default, the radio module has baudrate value rfm_baudrate, so we
    // set that value on the local UART port
    uart_init(UART_PORT, RFM_BAUDRATE);

    gpio_set_function(RFM_TX, GPIO_FUNC_UART);
    gpio_set_function(RFM_RX, GPIO_FUNC_UART);

    // Set UART format to 8 data bits, 1 stop bit, no parity
    uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);

    // Set flow control to false for both
    uart_set_hw_flow(UART_PORT, false, false);

    uart_set_fifo_enabled(UART_PORT, true);
    return true;
}

bool Radio::read(std::vector<Telemetry> &result)
{
    const size_t BUFFER_SIZE = 10000;
    // Telemetry packet size in bytes
    char data[BUFFER_SIZE];
    size_t end_index = 0;

#ifdef RATS_TIME
    const absolute_time_t start_time = get_absolute_time();
#endif

    // If there isn't incoming data after 1ms, the data packet is over
    const int timeout_us = 1000; // 1ms
    while (uart_is_readable_within_us(UART_PORT, timeout_us))
    {
        const char c = uart_getc(UART_PORT);

        // debug_log("Received character: %c\n", c);
        // Append the character to the response buffer if there's space
        if (end_index < BUFFER_SIZE)
        {
            data[end_index++] = c;
        }
        else
        {

            debug_log("Response buffer overflow: \n");
            break;
        }
    }

    // Store result if data was received
    const bool data_received = (end_index != 0);

    if (!data_received)
    {
        return false;
    }

    // printf("\n\n");

    // debug_log("Data size: %d\n", end_index);

    // See if enough data was received
    // if (data_index != RESPONSE_SIZE)
    // {
    //     debug_log("UART data of size %d was received, but not the correct size\n", data_index);
    //     return false;
    // }

    // print_buffer_hex(data, end_index);

    // TODO: Salvage a packet if it got split between two data burts
    size_t i = 0;
    while (i < end_index)
    {

        // Keep reading until you find sync number
        while (i + sizeof(SYNC_WORD) < end_index && !read_sync_word(data, i))
        {
            i++;
        }

        // If it's possible for the packet to have been fully delivered
        if (i + PACKET_SIZE <= end_index)
        {
            // Extract the packet
            Telemetry telemetry;
            std::memcpy(&telemetry, data + i + sizeof(SYNC_WORD), sizeof(Telemetry));

            // Store the packet
            result.push_back(telemetry);
        }
        i += PACKET_SIZE;
    }

#ifdef RATS_TIME
    // Calculate elapsed time in seconds
    const int64_t elapsed_us = absolute_time_diff_us(start_time, get_absolute_time());

    // Print the elapsed time using the PRId64 macro for 64-bit integers
    printf("Elapsed time: %" PRId64 " microseconds\n", elapsed_us);
#endif

    // printf("Number of telemetry packets processed: %d\n", result.size());

    if (result.size() == 0)
    {
        return false;
    }

    return true;
}