/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
// 0
// 1       @
// 2
// 3             @
// 4
// 5
// 6
// 7
//
// E.g. if slave addresses 0x12 and 0x34 were acknowledged.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "tusb.h"

#define TX_PIN 0
#define RX_PIN 1

#define UART_ID uart0
#define BAUD_RATE 9600


#define I2C_PORT i2c0

#define I2C_SDA 12
#define I2C_SCL 13
#define SDA_PIN 12
#define SCL_PIN 13

#define UBLOX_ADDR (0x42)

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

int main() {
    stdio_init_all();

    sleep_ms(10000);

    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }
    printf("Connected\n");


    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {

        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else {
            ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false); }

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");

    sleep_ms(500);

    return 0;
}

// int main() {
//     stdio_init_all();
//     sleep_ms(1000); // Allow time for USB serial to initialize

//     i2c_init(I2C_PORT, 400 * 1000);
//     gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
//     gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
//     gpio_pull_up(SDA_PIN);
//     gpio_pull_up(SCL_PIN);


//     while (!tud_cdc_connected()) {
//         sleep_ms(500);
//     }
//     printf("Connected\n");


//     // int devices_found = 0;
//     // for (uint8_t addr = 1; addr < 127; addr++) {
//     //     int ret = i2c_write_blocking(I2C_PORT, addr, NULL, 0, false);
//     //     printf("(%d) ", ret);
//     //     if (ret >= 0) {
//     //         printf("Found device at 0x%02X\n", addr);
//     //         devices_found++;
//     //     }
//     // }

//     // if (devices_found == 0) {
//     //     printf("No I2C devices found.\n");
//     // } else {
//     //     printf("Total devices found: %d\n", devices_found);
//     // }

//     // while (1) {
//     //     tight_loop_contents();
//     // }


//     uint8_t msg[17] = {0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x01, 0x01, 0x00, 0x00, 0x25, 0x00, 0x93, 0x10, 0x00, 0x63, 0xCC};
//     int ret = i2c_write_blocking(I2C_PORT, UBLOX_ADDR, msg, 17, false);
//     printf("ret value: %d\n", ret);

//     sleep_ms(500);

//     return 0;
// }