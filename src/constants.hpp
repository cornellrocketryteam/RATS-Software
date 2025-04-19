/**
 * @file constants.hpp
 * @author sj728
 *
 * @brief Constants and other definitions
 */
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include "stepper.hpp"
#include "units.hpp"
#include <cstddef>
#include <cstdint>

/**
 * Container for hardware RATS software interacts with
 */
namespace constants {

    // Radio
    constexpr unsigned int radioBaudrate = 115200;
    constexpr unsigned int radioDataBits = 8;
    constexpr unsigned int radioStopBits = 1;
    constexpr std::size_t radioReceiveBufferBytes = 10000;
    constexpr unsigned int radioBurstGapUs = rats::units::microsecondsPerMillisecond;
    // SD
    constexpr unsigned int sd_baudrate = 125 * 1000000 / 6; // baud

    /** NAV-PVT protocol values and the minimum accepted GNSS solution quality. */
    namespace gnss {
        constexpr std::uint8_t threeDimensionalFix = 3;
        constexpr std::uint8_t fixOkMask = 0x01;
        constexpr std::uint16_t invalidPositionMask = 0x0001;
        constexpr std::uint8_t validDateMask = 0x01;
        constexpr std::uint8_t validTimeMask = 0x02;
        constexpr std::uint8_t fullyResolvedTimeMask = 0x04;
        constexpr std::uint8_t requiredUtcValidityMask = validDateMask | validTimeMask | fullyResolvedTimeMask;
        constexpr double degreesPerCoordinateUnit = 1e-7;
        constexpr unsigned int minimumSatellites = 6;
        constexpr std::uint32_t maximumHorizontalAccuracyMm = 5000;
        constexpr std::uint32_t maximumVerticalAccuracyMm = 10000;
    } // namespace gnss

    /**
     * Antenna calibration and telemetry conventions. Verify these against the
     * installed board and flight software before enabling configurationVerified.
     */
    namespace tracking {
        constexpr bool configurationVerified = false;
        constexpr rats::AxisConfig azimuthConfig {};
        constexpr rats::AxisConfig elevationConfig {};
        constexpr double azimuthReferenceDeg = 0.0;
        constexpr double elevationReferenceDeg = 0.0;
        constexpr double azimuthAtZeroDeg = 0.0; // Clockwise from true north.
        constexpr double stationAntennaToPivotM = 0.0; // Pivot MSL minus GNSS antenna MSL.

        // Confirm the packet's coordinate scale, altitude datum and fix-valid flag.
        constexpr double rocketUnitsPerDegree = 0.0;
        constexpr double rocketAltitudeMetresPerUnit = 0.0;
        constexpr double rocketAltitudeToMslOffsetM = 0.0;
        constexpr std::uint16_t rocketFixValidMask = 0;
        constexpr std::uint16_t rocketFixValidValue = 0;

        constexpr unsigned int gnssBusHz = 100000;
        constexpr int gnssMeasurementMs = 100;
        constexpr std::uint64_t stationTimeoutUs = 60 * rats::units::microsecondsPerSecond;
        constexpr std::uint64_t staleAfterUs = 2 * rats::units::microsecondsPerSecond;
        constexpr std::int64_t maxSourceAgeSeconds = 2;
        constexpr std::int64_t futureSourceToleranceSeconds = 1;
        constexpr std::int64_t sourceTimestampUncertaintySeconds = 1;
        constexpr std::uint64_t motorMailboxIntervalUs = rats::units::microsecondsPerMillisecond;
        constexpr std::size_t motorStackWords = 2048;
        constexpr std::size_t motorStackAlignmentBytes = 8;
        constexpr std::size_t usbQueueCapacity = 16;
    } // namespace tracking

} // namespace constants

#endif // CONSTANTS_HPP
