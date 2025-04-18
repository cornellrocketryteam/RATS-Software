/**
 * @file main.cpp
 * @author sj728
 *
 * @brief The entry point for the program
 */

#include <cstdio>
#include "pico/stdlib.h"
#include "pins.hpp"
#include "tusb.h"
#include "sd.hpp"
#include "telemetry.hpp"
#include "radio.hpp"
#include "ublox_mx.hpp"
#include "ublox_nav_pvt.hpp"
#include "hardware/i2c.h"
#include "vector"

int main()
{
    stdio_init_all();

    Radio radio;
    if (!radio.start())
    {
        printf("Radio failed to start\n");
        // TODO: report radio failing
        return 1;
    }

    // GNSS gnss(i2c0);
    // if (!gnss.begin_PVT(100))
    // {
    //     printf("GNSS failed to start\n");
    //     return 1;
    // }

    // Initialize SD card module
    // Initialize GNSS module

#ifdef MOVER
    // Iniatialize Motor controller
#endif
    
    const int WAIT_TIME_MS = 1;
    while (true)
    {
        
        std::vector<Telemetry> telemetry_packets;

        bool success = radio.read(telemetry_packets);

        // Untested for now, but this is how data will be transferred serially
        // to the external software component (RATS proxy)
        if (success)
        {
            // Write the entire struct's raw memory to stdout in one call,
            // sending it serially to the external software component
            const int num_elements = 1;
            for (const Telemetry &telemetry : telemetry_packets)
            {
                fwrite(&telemetry, sizeof(Telemetry), num_elements, stdout);
                sleep_ms(WAIT_TIME_MS);
            }
        }

#ifdef MOVER
        // Move motors based on data from radio module
#endif
    }

    return 0;
}