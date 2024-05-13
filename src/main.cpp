
/*
 Rotational Antenna Tracking System Software
 Cornell Rocketry Team

 This program receives rocket telemetry,
 aims the antenna at the rocket's location,
 and passes the data on to the ground station laptop

 Created SP23
 by Zach Garcia

 */

#define SERVO
#define STEPPER
#define DEBUG

#include "../lib/pico-servo/include/pico_servo.h"
#include "constants.hpp"
#include "data.hpp"
#include "formulas.hpp"
#include "pico/stdlib.h"
#include "pins.hpp"
#include "rfm/pico_hal.h"
#include "stepper.h"
#include "tusb.h"
#include <RadioLib.h>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string>

#include "../lib/pico-servo/src/pico_servo.c"
#include "../lib/pico-stepper/lib/stepper.c"

bool launched = false;
bool recPacket = false;
long launchTime = 0;
int stepPos = 0;
float rockElev;
float rockLat;
float rockLong;

// RadioLib setup
PicoHal *hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK, 8000000);

SX1276 radio = new Module(hal, RX_CS, RX_DIO0, RADIOLIB_NC, RX_DIO1);

// initialize the stepper library on pins 7 through 10:

#ifdef STEPPER
stepper_t stepper;
#endif // STEPPER

int main() {
    stdio_init_all();

    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }

#ifdef SERVO
    // Initialize servo
    servo_init();
    servo_clock_auto();
    // Set duty cycle bounds in us
    servo_set_bounds(500, 2500);
    // Attach to pin 12
    servo_attach(constants::SERVO_PIN);
    // Set servo to 0 degrees
    servo_move_to(constants::SERVO_PIN, 0);
#endif // SERVO

#ifdef STEPPER
    // Initialize stepper
    stepper_init(&stepper, constants::STEPPER_PIN_1A, constants::STEPPER_PIN_1B, constants::STEPPER_PIN_2A, constants::STEPPER_PIN_2B, constants::STEPPER_STEPS_PER_REV, constants::STEPPING_MODE);
    stepper_set_speed_rpm(&stepper, constants::SPEED);
#endif // STEPPER

    gpio_init(RX_CS);
    gpio_set_dir(RX_CS, GPIO_OUT);

    gpio_init(RX_RST);
    gpio_set_dir(RX_RST, GPIO_OUT);

    sleep_ms(10);
    gpio_put(RX_RST, 0);
    sleep_ms(10);
    gpio_put(RX_RST, 1);

#ifdef DEBUG
    printf("[SX1276] Initializing ... ");
#endif // DEBUG

    int state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
    if (state != RADIOLIB_ERR_NONE) {
        printf("failed, code %d\n", state);
        return 1;
    }

#ifdef DEBUG
    printf("success!\n");
#endif // DEBUG

    while (true) {
        // receive a packet
#ifdef DEBUG
        printf("[SX1276] Waiting for incoming transmission ... ");
#endif // DEBUG
        uint8_t received[constants::MSG_LEN];
        int state = radio.receive(received, constants::MSG_LEN);

        if (state == RADIOLIB_ERR_NONE) {

#ifdef DEBUG
            // packet was successfully received
            printf("success!");
            printf("\n");
#endif // DEBUG
            std::string result = (char *)received;

            // Automatically Correct Frequency Error
            float rssi = radio.getRSSI();
            float snr = radio.getSNR();
            float freqErr = radio.getFrequencyError(true);

            // Package radio metadata
            char radio_metadata[sizeof(float) * 3];
            memcpy(radio_metadata, &rssi, sizeof(rssi));
            memcpy(radio_metadata + sizeof(float), &snr, sizeof(snr));
            memcpy(radio_metadata + (2 * sizeof(float)), &snr, sizeof(snr));

#ifdef DEBUG

            // print the RSSI (Received Signal Strength Indicator)
            // of the last received packet
            printf("[SX1278] RSSI:\t\t\t");
            printf("%f", rssi);
            printf(" dBm\n");

            // print the SNR (Signal-to-Noise Ratio)
            // of the last received packet
            printf("[SX1278] SNR:\t\t\t");
            printf("%f", snr);
            printf(" dB\n");

            // print frequency error
            // of the last received packet
            printf("[SX1278] Frequency error:\t");
            printf("%f", freqErr);
            printf(" Hz\n");
#endif // DEBUG
       // Send data to Ground Station
            for (uint i = 0; i < constants::MSG_LEN; i++) {
                printf("%c", received[i]);
            }
            for (uint i = 0; i < sizeof(radio_metadata); i++) {
                printf("%c", radio_metadata[i]);
            }

            // TODO(Zach) Check GPS valid flag

            // Extract values
            memcpy(&rockElev, received + constants::ALTITUDE_OFFSET, sizeof(float));  // Altitude field
            memcpy(&rockLat, received + constants::LATITUDE_OFFSET, sizeof(float));   // Latitude field
            memcpy(&rockLong, received + constants::LONGITUDE_OFFSET, sizeof(float)); // Longitude field

        } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
            // timeout occurred while waiting for a packet
#ifdef DEBUG
            printf("timeout!\n");
#endif // DEBUG
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            // packet was received, but is malformed
#ifdef DEBUG
            printf("CRC error!\n");
#endif // DEBUG
        } else if (state == RADIOLIB_ERR_SPI_WRITE_FAILED) {
            // Likely something on board shorted. Restart radio module
            int restart_state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
            while (restart_state != RADIOLIB_ERR_NONE) {
                printf("failed, code %d\n", state);
                restart_state = radio.begin(constants::FREQ, constants::BW, constants::SF, constants::CR, constants::SW, constants::PWR);
            }
        } else {
            // some other error occurred
#ifdef DEBUG
            printf("failed, code %d\n", state);
#endif // DEBUG
        }

        if (launched && launchTime == 0) {
            launchTime = to_ms_since_boot(get_absolute_time());
        } else if (launched && !recPacket) {
            // switch to backup data
            int dataIndex = (to_ms_since_boot(get_absolute_time()) - launchTime) / 500;
            rockElev = alt_data[dataIndex];
        }

        // convert coordinates to angles
        double dist = distance(rockLat, rockLong, constants::GROUND_LAT, constants::GROUND_LONG);
#ifdef DEBUG
        printf("Distance: ");
        printf("%d", dist);
        printf(" km\n");
#endif // DEBUG
        double bear = bearing(rockLat, rockLong, constants::GROUND_LAT, constants::GROUND_LONG);
#ifdef DEBUG
        printf("Bearing: ");
        printf("%d", bear);
        printf(" degrees\n");
#endif // DEBUG
        double asc = ascension(rockElev, constants::GROUND_ELEV, dist);
#ifdef DEBUG
        printf("Ascension: ");
        printf("%d", asc);
        printf(" degrees\n");
#endif // DEBUG

#ifdef STEPPER
        int target = -1 * (bear / 360) * constants::PLATFORM_STEPS_PER_REV;
        int stepsToTake = target - stepPos;
        if (target - stepPos > (constants::PLATFORM_STEPS_PER_REV / 2)) {
            stepsToTake = (target - stepPos) - constants::PLATFORM_STEPS_PER_REV;
        } else if (target - stepPos < -(constants::PLATFORM_STEPS_PER_REV / 2)) {
            stepsToTake = (target - stepPos) + constants::PLATFORM_STEPS_PER_REV;
        } else {
            stepsToTake = target - stepPos;
        }
        stepPos = target;
#endif // STEPPER
#ifdef SERVO
        // aim antenna
        servo_move_to(constants::SERVO_PIN, 90 - asc);
#endif // SERVO
#ifdef STEPPER
        stepper_rotate_steps(&stepper, stepsToTake);
        stepper_release(&stepper);
#endif // STEPPER
    }

    return 0;
}
