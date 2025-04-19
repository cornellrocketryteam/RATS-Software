/**
 * @file formulas.hpp
 * @author sj728
 *
 * @brief Coordinate calculations for the RATS antenna movement.
 */

#ifndef RATS_FORMULAS_HPP
#define RATS_FORMULAS_HPP

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include "units.hpp"

namespace rats {
    constexpr double pi = 3.14159265358979323846;
    /* Earth radius in meters used by the spherical distance calculation. */
    constexpr double earthRadiusM = 6371000.0;
    /* Minimum ground separation in meters for a defined pointing direction. */
    constexpr double minimumPointingDistanceM = 0.01;

    /**
     * Converts an angle from degrees to radians.
     */
    inline double toRadians(double degrees) {
        return degrees * pi / units::degreesPerHalfRevolution;
    }

    /**
     * Converts an angle from radians to degrees.
     */
    inline double toDegrees(double radians) {
        return radians * units::degreesPerHalfRevolution / pi;
    }

    /**
     * Wraps an angle to the range [-180, 180) degrees.
     */
    inline double wrap180(double degrees) {
        double angle = std::fmod(degrees + units::degreesPerHalfRevolution, units::degreesPerRevolution);
        if (angle < 0.0) angle += units::degreesPerRevolution;
        return angle - units::degreesPerHalfRevolution;
    }

    /**
     * Geographic position in degrees and meters. Station and rocket altitudes
     * must use the same altitude datum.
     */
    struct Position {
        double latitudeDeg;
        double longitudeDeg;
        double altitudeM;
    };

    /**
     * Checks latitude and longitude bounds and requires all fields to be finite.
     * @return True if the position is valid, false otherwise.
     */
    inline bool valid(Position p) {
        return std::isfinite(p.latitudeDeg) && std::abs(p.latitudeDeg) <= units::degreesPerQuarterRevolution &&
               std::isfinite(p.longitudeDeg) && std::abs(p.longitudeDeg) <= units::degreesPerHalfRevolution &&
               std::isfinite(p.altitudeM);
    }

    /**
     * Calculates the surface distance in meters between two positions supplied
     * in degrees, using the haversine formula.
     */
    inline double distance(double lat1, double lon1, double lat2, double lon2) {
        const double dLat = toRadians(lat2 - lat1);
        const double dLon = toRadians(lon2 - lon1);
        const double a = std::pow(std::sin(dLat / 2.0), 2) +
                         std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) *
                             std::pow(std::sin(dLon / 2.0), 2);
        return 2.0 * earthRadiusM * std::asin(std::sqrt(std::clamp(a, 0.0, 1.0)));
    }

    /**
     * Direction and surface distance from the station to the rocket.
     */
    struct Pointing {
        double azimuthDeg;   // Clockwise from true north. East is 90 degrees.
        double elevationDeg; // Upward from the horizon. Vertical is 90 degrees.
        double horizontalDistanceM;
    };

    /**
     * Calculates the bearing, elevation and surface distance to the rocket.
     * The local-range elevation model omits Earth curvature and refraction.
     * @throws std::invalid_argument If a position is invalid or the bearing is
     * undefined at coincident ground coordinates, including directly overhead.
     */
    inline Pointing pointing(Position station, Position rocket) {
        if (!valid(station) || !valid(rocket))
            throw std::invalid_argument("Invalid geographic position");
        const double lat1 = toRadians(station.latitudeDeg);
        const double lat2 = toRadians(rocket.latitudeDeg);
        const double dLon = toRadians(rocket.longitudeDeg - station.longitudeDeg);
        const double horizontal = distance(station.latitudeDeg, station.longitudeDeg, rocket.latitudeDeg, rocket.longitudeDeg);
        if (horizontal < minimumPointingDistanceM)
            throw std::invalid_argument("Undefined azimuth at coincident coordinates");
        const double y = std::sin(dLon) * std::cos(lat2);
        const double x = std::cos(lat1) * std::sin(lat2) -
                         std::sin(lat1) * std::cos(lat2) * std::cos(dLon);
        double azimuth = toDegrees(std::atan2(y, x));
        if (azimuth < 0.0) azimuth += units::degreesPerRevolution;
        return {azimuth, toDegrees(std::atan2(rocket.altitudeM - station.altitudeM, horizontal)), horizontal};
    }
} // namespace rats

#endif // RATS_FORMULAS_HPP
