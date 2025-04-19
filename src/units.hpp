/**
 * @file units.hpp
 *
 * @brief Shared angle, time and distance conversion constants.
 */

#ifndef RATS_UNITS_HPP
#define RATS_UNITS_HPP

#include <cstdint>

namespace rats::units {
    constexpr double degreesPerRevolution = 360.0;
    constexpr double degreesPerHalfRevolution = 180.0;
    constexpr double degreesPerQuarterRevolution = 90.0;

    constexpr std::uint64_t secondsPerMinute = 60;
    constexpr std::uint64_t minutesPerHour = 60;
    constexpr std::uint64_t hoursPerDay = 24;
    constexpr std::uint64_t secondsPerHour = secondsPerMinute * minutesPerHour;
    constexpr std::uint64_t secondsPerDay = secondsPerHour * hoursPerDay;
    constexpr std::uint64_t millisecondsPerSecond = 1000;
    constexpr std::uint64_t microsecondsPerMillisecond = 1000;
    constexpr std::uint64_t microsecondsPerSecond = millisecondsPerSecond * microsecondsPerMillisecond;

    constexpr double metresPerMillimetre = 1e-3;
} // namespace rats::units

#endif // RATS_UNITS_HPP
