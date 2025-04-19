/**
 * @file pins.hpp
 * @author sj728
 *
 * @brief GPIO pin definitions for the Pico
 */
#ifndef PINS_HPP
#define PINS_HPP

#define SD_CMD 3
#define SD_D0 4

// SD card reader GPIO pins
#define SPI_PORT spi0
#define SPI_MISO 16
#define SPI_MOSI 19
#define SPI_SCK 18
#define SPI_CS 17

// Radio GPIO Pins
#define UART_PORT uart1
#define RFM_TX 4
#define RFM_RX 5
#define RFM_BAUDRATE 115200

#ifdef __cplusplus
/**
 * Antenna GPIO assignments. Unassigned pins keep the motor integration disabled.
 * The guard preserves this header's use by the SD library's C configuration.
 */
namespace pins {
    constexpr int unassigned = -1;

    /** Standard Raspberry Pi Pico board pins and I2C0 GPIO function mapping. */
    namespace board {
        constexpr int lastAvailableGpio = 28;
        constexpr int smpsMode = 23;
        constexpr int vbusSense = 24;
        constexpr int led = 25;
        constexpr int i2cPinFunctionPeriod = 4;
        constexpr int i2c0SdaOffset = 0;
        constexpr int i2c0SclOffset = 1;
    } // namespace board

    struct MotorPins {
        int step, direction, enable, fault;
        bool enableActiveLow, faultActiveLow;
    };

    constexpr int gnssSda = unassigned;
    constexpr int gnssScl = unassigned;
    constexpr int arm = unassigned;       // Active low. An open switch disarms.
    constexpr int reference = unassigned; // Active-low manual alignment confirmation.
    constexpr MotorPins azimuth {unassigned, unassigned, unassigned, unassigned, true, true};
    constexpr MotorPins elevation {unassigned, unassigned, unassigned, unassigned, true, true};
} // namespace pins
#endif

#endif // PINS_HPP
