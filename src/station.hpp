/**
 * @file station.hpp
 *
 * @brief Ground station acquisition and rocket telemetry coordinates.
 */
#ifndef STATION_HPP
#define STATION_HPP

#include "formulas.hpp"
#include "mover.hpp"
#include "telemetry.hpp"
#include <cstdint>
#include <optional>

struct UbxNavPvt;

namespace rats {

    /**
     * Station position and the UTC reference captured with its GNSS fix.
     * The station remains fixed until the next acquisition.
     */
    struct StationReference {
        Position position {};
        std::uint64_t unixSeconds = 0;
        std::uint64_t capturedAtUs = 0;
    };

    /**
     * Rocket position with an independent source-age deadline.
     */
    struct TrackingTarget {
        TelemetryFix fix {};
        std::uint64_t expiresAtUs = 0;
    };

    /** Validates a NAV-PVT fix and converts its coordinates and UTC date. */
    std::optional<StationReference> stationFromPvt(const UbxNavPvt &pvt, std::uint64_t nowUs);

    /** Configures I2C GNSS and waits for a valid stationary reference. */
    std::optional<StationReference> acquireStation();

    /** Converts a radio packet to a fresh target using the station's MSL datum. */
    TrackingTarget trackingFromTelemetry(const Telemetry &packet, const StationReference &station,
                                        std::uint64_t nowUs);

} // namespace rats

#endif // STATION_HPP
