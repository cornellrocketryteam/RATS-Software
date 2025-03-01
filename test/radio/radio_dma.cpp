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

#include "hardware/dma.h" // For DMA

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

const size_t PACKET_SIZE = 102 + sizeof(SYNC_WORD);

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

uint8_t rx_buffer[PACKET_SIZE * 2];    // Buffer to store received data
volatile bool packet_received = false; // Flag to indicate packet completion
int dma_chan;
dma_channel_config dma_config;

volatile int rx_offset = 0; // Track received bytes

void dma_handler()
{
    printf("DMA interrupt detected\n");

    // printf("RX Buffer: \n");
    // print_buffer_hex((const char *)rx_buffer, PACKET_SIZE);

    // Clear DMA interrupt flag
    dma_hw->ints0 = 1u << dma_chan;

    // Check how many bytes have been received so far
    rx_offset += 32; // Increment by 32-byte FIFO size

    if (rx_offset > 80)
    {
        dma_channel_set_write_addr(dma_chan, rx_buffer, false); // Set write address back to start of buffer
        packet_received = true;                                 // Full packet received
        rx_offset = 0;                                          // Reset offset for next packet
    }
    else if (rx_offset >= PACKET_SIZE)
    {
        packet_received = true; // Full packet received
        rx_offset = 0;          // Reset offset for next packet
    }

    // Restart DMA for the next chunk, **but only if we're not at the start of a new packet**
    // if (!packet_received)
    if (true)
    {
        dma_channel_set_read_addr(dma_chan, &uart_get_hw(UART_PORT)->dr, true);
    }
    // dma_channel_set_read_addr(dma_chan, &uart_get_hw(UART_PORT)->dr, false);
    printf("DMA interrupt handled\n\n");
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

    uart_set_fifo_enabled(UART_PORT, true); // Enable FIFO for efficient DMA transfers

    // DMA Setup
    dma_chan = dma_claim_unused_channel(true);
    dma_config = dma_channel_get_default_config(dma_chan);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_config, false);
    channel_config_set_write_increment(&dma_config, true);
    channel_config_set_dreq(&dma_config, uart_get_dreq(UART_PORT, false));

    dma_channel_configure(
        dma_chan,
        &dma_config,
        rx_buffer,                   // Buffer to store packet
        &uart_get_hw(UART_PORT)->dr, // Read from UART RX FIFO
        32,                          // FIFO size of 32 bytes
        false                        // Don't start yet
    );

    // Enable DMA IRQ
    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

    // Start DMA transfer
    dma_start_channel_mask(1u << dma_chan);

    return true;
}

bool Radio::read(std::vector<Telemetry> &result)
{
    // dma_channel_set_irq0_enabled(dma_chan, false);
    {
        printf("Reading radio data\n");
        // Wait until the DMA transfer completes
        if (!packet_received)
        {
            printf("No packet received yet\n\n");
            return false; // No new packet received yet
        }

        packet_received = false; // Reset flag

        // Copy the received data into the result structure
        // Telemetry telemetry;
        // std::memcpy(&telemetry, rx_buffer, sizeof(Telemetry));
        print_buffer_hex((const char *)rx_buffer, PACKET_SIZE);

        // Restart DMA transfer for the next packet
        rx_offset = 0; // Reset offset
        dma_channel_set_read_addr(dma_chan, &uart_get_hw(UART_PORT)->dr, true);
    }
    // dma_channel_set_irq0_enabled(dma_chan, true);

    return true;
}

/*******************/
/* OLD Radio Code */
/******************/

// bool Radio::start()
// {
//     // By default, the radio module has baudrate value rfm_baudrate, so we
//     // set that value on the local UART port
//     uart_init(UART_PORT, RFM_BAUDRATE);

//     gpio_set_function(RFM_TX, GPIO_FUNC_UART);
//     gpio_set_function(RFM_RX, GPIO_FUNC_UART);

//     // Set UART format to 8 data bits, 1 stop bit, no parity
//     uart_set_format(UART_PORT, 8, 1, UART_PARITY_NONE);

//     // Set flow control to false for both
//     uart_set_hw_flow(UART_PORT, false, false);

//     uart_set_fifo_enabled(UART_PORT, true);
//     return true;
// }

// bool Radio::read(std::vector<Telemetry> &result)
// {
//     const size_t BUFFER_SIZE = 10000;
//     // Telemetry packet size in bytes
//     char data[BUFFER_SIZE];
//     size_t end_index = 0;

// #ifdef RATS_TIME
//     const absolute_time_t start_time = get_absolute_time();
// #endif

//     // If there isn't incoming data after 1ms, the data packet is over
//     const int timeout_us = 1000; // 1ms
//     while (uart_is_readable_within_us(UART_PORT, timeout_us))
//     {
//         const char c = uart_getc(UART_PORT);

//         // debug_log("Received character: %c\n", c);
//         // Append the character to the response buffer if there's space
//         if (end_index < BUFFER_SIZE)
//         {
//             data[end_index++] = c;
//         }
//         else
//         {

//             debug_log("Response buffer overflow: \n");
//             break;
//         }
//     }

//     // Store result if data was received
//     const bool data_received = (end_index != 0);

//     if (!data_received)
//     {
//         return false;
//     }

//     // printf("\n\n");

//     // debug_log("Data size: %d\n", end_index);

//     // See if enough data was received
//     // if (data_index != RESPONSE_SIZE)
//     // {
//     //     debug_log("UART data of size %d was received, but not the correct size\n", data_index);
//     //     return false;
//     // }

//     // print_buffer_hex(data, end_index);

//     // TODO: Salvage a packet if it got split between two data burts
//     size_t i = 0;
//     while (i < end_index)
//     {

//         // Keep reading until you find sync number
//         while (i + sizeof(SYNC_WORD) < end_index && !read_sync_word(data, i))
//         {
//             i++;
//         }

//         // If it's possible for the packet to have been fully delivered
//         if (i + PACKET_SIZE <= end_index)
//         {
//             // Extract the packet
//             Telemetry telemetry;
//             std::memcpy(&telemetry, data + i + sizeof(SYNC_WORD), sizeof(Telemetry));

//             // Store the packet
//             result.push_back(telemetry);
//         }
//         i += PACKET_SIZE;
//     }

// #ifdef RATS_TIME
//     // Calculate elapsed time in seconds
//     const int64_t elapsed_us = absolute_time_diff_us(start_time, get_absolute_time());

//     // Print the elapsed time using the PRId64 macro for 64-bit integers
//     printf("Elapsed time: %" PRId64 " microseconds\n", elapsed_us);
// #endif

//     // printf("Number of telemetry packets processed: %d\n", result.size());

//     if (result.size() == 0)
//     {
//         return false;
//     }

//     return true;
// }