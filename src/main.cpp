
/*
 Rotational Antenna Tracking System Software
 Cornell Rocketry Team

 This program receives rocket telemetry,
 aims the antenna at the rocket's location,
 and passes the data on to the ground station laptop

 Created SP23
 by Zach Garcia

 */

// Uncomment if you want debug information about signal strength and motor movements
// #define DEBUG

// #include <Stepper.h>
// #include <Servo.h>
// #include <ArduinoJson.h>
#include "../lib/RS-FEC.h"
#include "../lib/pico-servo/include/pico_servo.h"
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
float groundElev = 0.0;
float groundLat = 0.0;
float groundLong = 0.0;
int stepPos = 0;
float rockElev;
float rockLat;
float rockLong;

// RadioLib setup
PicoHal *hal = new PicoHal(SPI_PORT, SPI_MISO, SPI_MOSI, SPI_SCK, 8000000);

SX1276 radio = new Module(hal, RX_CS, RX_DIO0, RADIOLIB_NC, RX_DIO1);

const int msglen = 61;
const uint8_t ECC_LENGTH = 50;
char encoded[msglen + ECC_LENGTH];
char repaired[msglen];

RS::ReedSolomon<msglen, ECC_LENGTH> rs;

// initialize the stepper library on pins 7 through 10:
stepper_t stepper;
const uint8_t stepper_pin_1A = 7;
const uint8_t stepper_pin_1B = 8;
const uint8_t stepper_pin_2A = 9;
const uint8_t stepper_pin_2B = 10;
const uint16_t stepper_steps_per_revolution = 200;
const uint16_t platformStepsPerRev = 1320;
const stepper_mode_t stepping_mode = single;
uint8_t speed = 60;

int main() {
    stdio_init_all();

    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }

    // Initialize servo
    servo_init();
    servo_clock_auto();
    // Set duty cycle bounds in us
    servo_set_bounds(500, 2500);
    // Attach to pin 12
    servo_attach(12);
    // Set servo to 0 degrees
    servo_move_to(12, 0);

    // Initialize stepper
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B, stepper_pin_2A, stepper_pin_2B, stepper_steps_per_revolution, stepping_mode);
    stepper_set_speed_rpm(&stepper, speed);

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
#endif

    int state = radio.begin();
    if (state != RADIOLIB_ERR_NONE) {
        printf("failed, code %d\n", state);
        return 1;
    }
#ifdef DEBUG
    printf("success!\n");
#endif

    while (true) {
        // receive a packet
#ifdef DEBUG
        printf("[SX1276] Waiting for incoming transmission ... ");
#endif

        uint8_t encoded[msglen + ECC_LENGTH];
        int state = radio.receive(encoded, msglen + ECC_LENGTH);

        if (state == RADIOLIB_ERR_NONE) {
#ifdef DEBUG
            // packet was successfully received
            printf("success!");
            // print the data of the packet
            printf("\n[SX1276] Raw Data Length: %d\n[SX1276] Raw Data: ", sizeof(encoded));
            for (uint i = 0; i < sizeof(encoded); i++) {
                printf("%c", encoded[i]);
            }
            printf("\n");
#endif

            rs.Decode(encoded, repaired);
            std::string result = repaired;
#ifdef DEBUG
            printf("Result: \"");
            for (int i = 0; i < msglen; i++)
                printf("%c", (char)repaired[i]);
            printf("\"\n");

            // print the RSSI (Received Signal Strength Indicator)
            // of the last received packet
            printf("[SX1278] RSSI:\t\t\t");
            printf("%f", radio.getRSSI());
            printf(" dBm\n");

            // print the SNR (Signal-to-Noise Ratio)
            // of the last received packet
            printf("[SX1278] SNR:\t\t\t");
            printf("%f", radio.getSNR());
            printf(" dB\n");

            // print frequency error
            // of the last received packet
            printf("[SX1278] Frequency error:\t");
            printf("%f", radio.getFrequencyError());
            printf(" Hz\n");
#endif

            // Send data to Ground Station
            printf("%s", repaired);

            // Extract values
            memcpy(&rockElev, repaired + 9, 4);  // Altitude field
            memcpy(&rockLat, repaired + 13, 4);  // Latitude field
            memcpy(&rockLong, repaired + 17, 4); // Longitude field

        } else if (state == RADIOLIB_ERR_RX_TIMEOUT) {
            // timeout occurred while waiting for a packet
            printf("timeout!\n");
        } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
            // packet was received, but is malformed
            printf("CRC error!\n");
        } else {
            // some other error occurred
            printf("failed, code %d\n", state);
        }

        if (launched && launchTime == 0) {
            launchTime = to_ms_since_boot(get_absolute_time());
        } else if (launched && !recPacket) {
            // switch to backup data
            int dataIndex = (to_ms_since_boot(get_absolute_time()) - launchTime) / 500;
            rockElev = alt_data[dataIndex];

        } else if (!recPacket) {
            break;
        }

        // convert coordinates to angles
        double dist = distance(rockLat, rockLong, groundLat, groundLong);
#ifdef DEBUG
        printf("Distance: ");
        printf("%d", dist);
        printf(" km\n");
#endif
        double bear = bearing(rockLat, rockLong, groundLat, groundLong);
#ifdef DEBUG
        printf("Bearing: ");
        printf("%d", bear);
        printf(" degrees\n");
#endif
        double asc = ascension(rockElev, groundElev, dist);
#ifdef DEBUG
        printf("Ascension: ");
        printf("%d", asc);
        printf(" degrees\n");
#endif
        // // Disabled since we removed the stepper motor
        // int target = -1 * (bear / 360) * platformStepsPerRev;
        // int stepsToTake = target - stepPos;
        // if (target - stepPos > (platformStepsPerRev / 2)) {
        //   stepsToTake = (target - stepPos) - platformStepsPerRev;
        // } else if (target - stepPos < -(platformStepsPerRev / 2)) {
        //   stepsToTake = (target - stepPos) + platformStepsPerRev;
        // } else {
        //   stepsToTake = target - stepPos;
        // }
        // stepPos = target;

        // aim antenna
        servo_move_to(12, 90 - asc);
        // stepper_rotate_steps(&stepper, stepsToTake);
        // stepper_release(&stepper);
        sleep_ms(500);
    }

    return 0;
}
