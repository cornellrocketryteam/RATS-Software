/**
 * @file pins.hpp
 * @author sj728
 *
 * @brief GPIO pin definitions for the Pico
 */
#ifndef PINS_HPP
#define PINS_HPP

// #define SD_CMD 3
// #define SD_D0 4

// SD card reader pins
#define SPI_PORT spi0
#define SPI_MISO 16
#define SPI_MOSI 19
#define SPI_SCK 18
#define SPI_CS 17

// Radio Pins
#define UART_PORT uart1
#define RFM_TX 4
#define RFM_RX 5
#define RFM_BAUDRATE 115200
#endif // PINS_HPP