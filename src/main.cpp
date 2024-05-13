
/*
 Rotational Antenna Tracking System Software
 Cornell Rocketry Team

 This program receives rocket telemetry,
 aims the antenna at the rocket's location,
 and passes the data on to the ground station laptop

 Created SP23
 by Zach Garcia

 */

// #define SERVO
// #define STEPPER
// #define DEBUG

#include "../lib/pico-servo/include/pico_servo.h"
#include "data.hpp"
#include "formulas.hpp"
#include "pico/stdlib.h"
#include "radio.hpp"
#include "stepper.h"
#include "tusb.h"
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
Radio radio;

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

    if (radio.init()) {
        return 1;
    };
    char received[constants::MSG_LEN];
    char radio_metadata[sizeof(float) * 3];

    while (true) {
        // receive a packet
        if (!radio.receive(received, radio_metadata)) {
            std::string result = (char *)received;

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
        }

        // Switch to backup data if needed
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
