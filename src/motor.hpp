/**
 * @file motor.hpp
 *
 * @brief Motor-core ownership and telemetry handoff for antenna tracking.
 */

#ifndef RATS_MOTOR_HPP
#define RATS_MOTOR_HPP

#include "station.hpp"
#include "telemetry.hpp"
#include <cstdint>

namespace rats::motor {

    /**
     * Validates the complete tracking configuration before configuring GPIO.
     * Keeps both motor drivers disabled while the station reference is acquired.
     * Call from core 0 before initializing the GNSS receiver.
     * @return True when configuration and GPIO preparation are complete.
     */
    bool prepare();

    /**
     * Starts core 1 with a fixed station reference after prepare() succeeds.
     * Core 1 exclusively owns STEP/DIR outputs and driver fault checks.
     * Call once from core 0; moving the station requires a reset and reacquisition.
     */
    void start(const StationReference &station);

    /**
     * Publishes the newest radio packet for core 1 using a protected mailbox.
     * Invalid or expired packets stop movement. Call from core 0 after start().
     * @param nowUs Local monotonic arrival time in microseconds.
     */
    void update(const Telemetry &packet, std::uint64_t nowUs);

} // namespace rats::motor

#endif // RATS_MOTOR_HPP
