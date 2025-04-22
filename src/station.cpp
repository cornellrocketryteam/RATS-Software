/**
 * @file station.cpp
 *
 * @brief GNSS reference validation and radio telemetry adaptation.
 */

#include "station.hpp"
#include "constants.hpp"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "pins.hpp"
#include "ublox_mx.hpp"
#include "ublox_nav_pvt.hpp"
#include <algorithm>
#include <array>

namespace {
    namespace gnss = constants::gnss;
    namespace tracking = constants::tracking;

    namespace calendar {
        constexpr unsigned int unixEpochYear = 1970;
        constexpr unsigned int firstSupportedYear = 2000;
        constexpr unsigned int lastSupportedYear = 2099;
        constexpr unsigned int leapYearPeriodYears = 4;
        constexpr unsigned int yearsPerCentury = 100;
        constexpr unsigned int gregorianCycleYears = 400;
        constexpr unsigned int daysPerCommonYear = 365;
        constexpr unsigned int daysPerLeapYear = 366;
        constexpr unsigned int februaryIndex = 1;
        constexpr unsigned int leapFebruaryDays = 29;
        constexpr std::array commonYearMonthDays {31u, 28u, 31u, 30u, 31u, 30u,
                                                  31u, 31u, 30u, 31u, 30u, 31u};
    } // namespace calendar

    bool leapYear(unsigned year) {
        return year % calendar::leapYearPeriodYears == 0 &&
               (year % calendar::yearsPerCentury != 0 || year % calendar::gregorianCycleYears == 0);
    }
} // namespace

namespace rats {

    std::optional<StationReference> stationFromPvt(const UbxNavPvt &pvt, std::uint64_t nowUs) {
        if (pvt.fixType != gnss::threeDimensionalFix || (pvt.flags & gnss::fixOkMask) == 0 ||
            (pvt.flags3 & gnss::invalidPositionMask) != 0 ||
            (pvt.valid & gnss::requiredUtcValidityMask) != gnss::requiredUtcValidityMask ||
            pvt.numSV < gnss::minimumSatellites || pvt.hAcc > gnss::maximumHorizontalAccuracyMm ||
            pvt.vAcc > gnss::maximumVerticalAccuracyMm ||
            pvt.year < calendar::firstSupportedYear || pvt.year > calendar::lastSupportedYear ||
            pvt.month == 0 || pvt.month > calendar::commonYearMonthDays.size() ||
            pvt.hour >= units::hoursPerDay || pvt.min >= units::minutesPerHour ||
            pvt.sec >= units::secondsPerMinute)
            return std::nullopt;

        auto monthDays = calendar::commonYearMonthDays;
        if (leapYear(pvt.year)) monthDays[calendar::februaryIndex] = calendar::leapFebruaryDays;
        if (pvt.day == 0 || pvt.day > monthDays[pvt.month - 1]) return std::nullopt;

        // Use 64-bit arithmetic for UTC seconds beyond the 32-bit Unix epoch range.
        std::uint64_t days = 0;
        for (unsigned year = calendar::unixEpochYear; year < pvt.year; ++year)
            days += leapYear(year) ? calendar::daysPerLeapYear : calendar::daysPerCommonYear;
        for (unsigned month = 1; month < pvt.month; ++month)
            days += monthDays[month - 1];
        days += pvt.day - 1;

        // NAV-PVT coordinates use 1e-7 degrees; hMSL uses millimetres.
        const Position position {pvt.lat * gnss::degreesPerCoordinateUnit,
                                 pvt.lon * gnss::degreesPerCoordinateUnit,
                                 pvt.hMSL * units::metresPerMillimetre + tracking::stationAntennaToPivotM};
        if (!valid(position)) return std::nullopt;
        return StationReference {position,
                                 days * units::secondsPerDay + pvt.hour * units::secondsPerHour +
                                     pvt.min * units::secondsPerMinute + pvt.sec,
                                 nowUs};
    }

    std::optional<StationReference> acquireStation() {
        i2c_init(i2c0, tracking::gnssBusHz);
        gpio_set_function(pins::gnssSda, GPIO_FUNC_I2C);
        gpio_set_function(pins::gnssScl, GPIO_FUNC_I2C);
        gpio_pull_up(pins::gnssSda);
        gpio_pull_up(pins::gnssScl);

        GNSS gnss(i2c0);
        // begin_PVT's implementation takes milliseconds, despite its header's Hz wording.
        if (!gnss.begin_PVT(tracking::gnssMeasurementMs)) return std::nullopt;

        const auto startedAtUs = time_us_64();
        while (time_us_64() - startedAtUs < tracking::stationTimeoutUs) {
            UbxNavPvt pvt {};
            if (gnss.read_PVT_data(&pvt)) {
                auto station = stationFromPvt(pvt, time_us_64());
                if (station) return station;
            }
            sleep_ms(tracking::gnssMeasurementMs);
        }
        return std::nullopt;
    }

    TrackingTarget trackingFromTelemetry(const Telemetry &packet, const StationReference &station,
                                        std::uint64_t nowUs) {
        TrackingTarget target;
        if (nowUs < station.capturedAtUs || tracking::rocketUnitsPerDegree <= 0 ||
            tracking::rocketAltitudeMetresPerUnit <= 0 || tracking::rocketFixValidMask == 0)
            return target;

        target.fix = {{packet.gps_latitude / tracking::rocketUnitsPerDegree,
                       packet.gps_longitude / tracking::rocketUnitsPerDegree,
                       packet.altitude * tracking::rocketAltitudeMetresPerUnit +
                           tracking::rocketAltitudeToMslOffsetM},
                      nowUs,
                      false};
        const auto utcNow = station.unixSeconds + (nowUs - station.capturedAtUs) / units::microsecondsPerSecond;
        const auto age = static_cast<std::int64_t>(utcNow) - packet.unix_time;
        if ((packet.metadata & tracking::rocketFixValidMask) != tracking::rocketFixValidValue ||
            packet.gps_num_satellites < gnss::minimumSatellites || !valid(target.fix.rocket) ||
            packet.unix_time == 0 || age < -tracking::futureSourceToleranceSeconds ||
            age >= tracking::maxSourceAgeSeconds)
            return target;

        // Account for whole-second timestamps and GNSS read latency.
        const auto remainingSeconds =
            tracking::maxSourceAgeSeconds - std::max<std::int64_t>(0, age) -
            tracking::sourceTimestampUncertaintySeconds;
        if (remainingSeconds <= 0) return target;
        target.fix.fixValid = true;
        target.expiresAtUs = nowUs + static_cast<std::uint64_t>(remainingSeconds) * units::microsecondsPerSecond;
        return target;
    }

} // namespace rats
