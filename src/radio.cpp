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

bool Radio::read(Telemetry *result)
{
    // Telemetry packet size in bytes
    const int RESPONSE_SIZE = 107;
    char data[RESPONSE_SIZE];
    size_t data_index = 0;

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
        if (data_index < RESPONSE_SIZE)
        {
            data[data_index++] = c;
        }
        else
        {
            return false;
        }
    }

    // Store result if data was received
    const bool data_received = (data_index != 0);
    if (!data_received)
    {
        return false;
    }

    // See if enough data was received
    if (data_index != RESPONSE_SIZE)
    {
        debug_log("UART data of size %d was received, but not the correct size\n", data_index);
        return false;
    }

#ifdef RATS_TIME
    // Calculate elapsed time in seconds
    const int64_t elapsed_us = absolute_time_diff_us(start_time, get_absolute_time());

    // Print the elapsed time using the PRId64 macro for 64-bit integers
    printf("Elapsed time: %" PRId64 " microseconds\n", elapsed_us);
#endif

    std::memcpy(result, data, sizeof(Telemetry));
    return true;
}