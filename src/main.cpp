/**
 * @file main.cpp
 * @author sj728
 *
 * @brief Receives and logs telemetry, forwards it to the proxy and updates RATS.
 */

#include "motor.hpp"
#include "pico/stdlib.h"
#include "radio.hpp"
#include "sd.hpp"
#include "station.hpp"
#include "usb.hpp"
#include <vector>

int main() {
    if (!rats::motor::prepare()) return 1;
    const auto station = rats::acquireStation();

    Radio radio;
    if (!radio.start()) return 1;

    SD sd;
    bool loggingHealthy = sd.begin();
    static rats::UsbForwarder usb;
    if (!usb.begin()) return 1;

    // Without a station fix, telemetry reception continues with motors disabled.
    if (station) rats::motor::start(*station);

    std::vector<Telemetry> packets;
    while (true) {
        packets.clear();
        if (radio.read(packets)) {
            const auto receivedAtUs = time_us_64();
            // Track the newest fix in a burst, but forward and log every packet.
            if (station && !packets.empty())
                rats::motor::update(packets.back(), receivedAtUs);

            for (const auto &packet : packets) {
                usb.service();
                usb.enqueue(packet);
                if (loggingHealthy && !sd.log_telemetry(packet))
                    loggingHealthy = false;
                usb.service();
            }
        }
        usb.service();
        tight_loop_contents();
    }
}
