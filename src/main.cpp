
/*
 Rotational Antenna Tracking System Software
 Cornell Rocketry Team

 This program receives rocket telemetry,
 aims the antenna at the rocket's location,
 and passes the data on to the ground station laptop

 Created SP23
 by Zach Garcia

 */

// #define MOVER
// #define DEBUG

#include "radio.hpp"
#include "mover.hpp"
#include "tusb.h"
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <string>


float rockElev;
float rockLat;
float rockLong;

// RadioLib setup
Radio radio;



#ifdef MOVER
// Mover setup
Mover mover;
#endif // MOVER

int main() {
    stdio_init_all();

    while (!tud_cdc_connected()) {
        sleep_ms(500);
    }

    if (radio.init()) {
        return 1;
    };

#ifdef MOVER
    // Mover initialization
    mover.init();
#endif // MOVER
    
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

            
#ifdef MOVER
            // Move
            mover.move(rockElev, rockLat, rockLong);
#endif // Mover
        }
    }

    return 0;
}
