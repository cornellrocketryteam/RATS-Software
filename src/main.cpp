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
        #ifdef RATS_VERBOSE
            printf("Radio failed to start\n");
        #endif
        return 1;
    }

    // Initialize GNSS module
    // GNSS gnss(i2c0);
    // if (!gnss.begin_PVT(100))
    // {
    //     printf("GNSS failed to start\n");
    //     return 1;
    // }

    
    // Initialize SD card module
    SD sd;
    if (!sd.begin())
    {
        #ifdef RATS_VERBOSE
            printf("SD card failed to start\n");
        #endif
        return 1;
    }

#ifdef MOVER
    // Iniatialize Motor controller
#endif

    int LED = 25;
    gpio_init(LED);

    gpio_set_dir(LED, GPIO_OUT);
    bool on = true;

    const int WAIT_TIME_MS = 2;
    while (true)
    {
        gpio_put(LED, on);
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
                sd.log_telemetry(telemetry);
                // fwrite(&telemetry, sizeof(Telemetry), num_elements, stdout);
                // fflush(stdout);
                printf("Telemetry: %u\n", telemetry.unix_time);
                

                sleep_ms(WAIT_TIME_MS);
            }
            on = !on;
        }

    
#ifdef MOVER
        // Move motors based on data from radio module
#endif

    }

    return 0;
}