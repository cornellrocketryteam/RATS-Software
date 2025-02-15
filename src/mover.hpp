/**
 * @file mover.hpp
 * @author sj728
 *
 * @brief Coordinates antenna elevation and rotation for the RATS antenna.
 */

#ifndef RATS_MOVER_HPP
#define RATS_MOVER_HPP

#include "formulas.hpp"
#include "stepper.hpp"
#include <cstdint>

namespace rats {

    /**
     * Rocket position, local arrival time and fix status used by the controller.
     */
    struct TelemetryFix {
        Position rocket;
        std::uint64_t receivedAtUs; // Local monotonic arrival time in microseconds, not rocket uptime.
        bool fixValid;
    };

    /**
     * Class for controlling antenna rotation and elevation from rocket positions.
     */
    class Mover {
    public:
        /**
         * Initializes the controller with the station position and mount heading.
         * @param azimuthAtZeroDeg True-north bearing of the rotator zero position.
         * @param staleAfterUs Maximum accepted telemetry age in microseconds.
         */
        Mover(Stepper &rotator, Stepper &elevator, Position station, double azimuthAtZeroDeg, std::uint64_t staleAfterUs)
            : rotator_(rotator), elevator_(elevator), station_(station),
              azimuthAtZeroDeg_(azimuthAtZeroDeg), staleAfterUs_(staleAfterUs) {
            if (!valid(station) || !std::isfinite(azimuthAtZeroDeg) || staleAfterUs == 0)
                throw std::invalid_argument("Invalid station configuration");
        }

        /**
         * Sets both motor targets from a valid, recent rocket position.
         * Invalid, future, stale or older-than-accepted fixes hold both axes.
         * @param nowUs Current local monotonic time in microseconds.
         * @return True if both targets were accepted, false otherwise.
         */
        bool move(TelemetryFix fix, std::uint64_t nowUs) {
            if (!fix.fixValid || !valid(fix.rocket) || fix.receivedAtUs > nowUs ||
                nowUs - fix.receivedAtUs > staleAfterUs_ ||
                (haveAcceptedFix_ && fix.receivedAtUs < latestReceivedUs_)) {
                hold();
                return false;
            }
            try {
                const Pointing aim = pointing(station_, fix.rocket);
                const double relativeAz = wrap180(aim.azimuthDeg - azimuthAtZeroDeg_);
                // Reject the whole command if either axis would exceed its bounds.
                if (!rotator_.canMove(relativeAz) || !elevator_.canMove(aim.elevationDeg)) {
                    hold();
                    return false;
                }
                rotator_.move(relativeAz);
                elevator_.move(aim.elevationDeg);
                latestReceivedUs_ = fix.receivedAtUs;
                haveAcceptedFix_ = true;
                tracking_ = true;
                return true;
            } catch (const std::invalid_argument &) {
                hold();
                return false;
            }
        }

        /**
         * Checks the home references and telemetry age, then advances both axes.
         * Call repeatedly with local monotonic time in microseconds.
         */
        void tick(std::uint64_t nowUs) {
            if (!rotator_.homed() || !elevator_.homed() ||
                (tracking_ && (nowUs < latestReceivedUs_ ||
                               nowUs - latestReceivedUs_ > staleAfterUs_))) hold();
            rotator_.tick(nowUs);
            elevator_.tick(nowUs);
        }

        /**
         * @return True while tracking an accepted fix, including at a settled target.
         */
        bool tracking() const {
            return tracking_;
        }

    private:
        /**
         * Stops new movement without clearing the accepted timestamp high-water mark.
         */
        void hold() {
            rotator_.hold();
            elevator_.hold();
            tracking_ = false;
        }
        /* Rotates the antenna left and right relative to the mount heading. */
        Stepper &rotator_;

        /* Raises and lowers the antenna relative to the horizon. */
        Stepper &elevator_;
        Position station_;
        double azimuthAtZeroDeg_;
        std::uint64_t staleAfterUs_, latestReceivedUs_ = 0;
        bool tracking_ = false, haveAcceptedFix_ = false;
    };
} // namespace rats

#endif // RATS_MOVER_HPP
