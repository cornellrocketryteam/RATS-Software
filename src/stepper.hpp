/**
 * @file stepper.hpp
 * @author sj728
 *
 * @brief Stepper motor control for the RATS antenna movement.
 */

#ifndef RATS_STEPPER_HPP
#define RATS_STEPPER_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "units.hpp"

namespace rats {

    /**
     * Interface to the motor driver enable, direction and step outputs.
     * The simulation supplies a driver that records these output changes.
     */
    struct StepDirDriver {
        virtual ~StepDirDriver() = default;
        virtual void enable(bool on) = 0;
        virtual void direction(bool positiveElectricalDirection) = 0;
        virtual void step(bool high) = 0;
    };

    /**
     * Motor scale, measured travel limits and STEP/DIR timing requirements.
     * Values must come from the installed motor, driver and antenna assembly.
     */
    struct AxisConfig {
        double fullStepsPerMotorRev;
        double microsteps;
        double motorRevsPerOutputRev;
        double minDegrees;
        double maxDegrees;
        std::uint64_t pulsePeriodUs;
        std::uint64_t pulseHighUs;
        std::uint64_t directionSetupUs;
        bool invertDirection;
    };

    /**
     * Class for controlling one stepper axis using scheduled STEP/DIR outputs.
     * Position is estimated from commanded steps without encoder feedback.
     */
    class Stepper {
    public:
        /**
         * Initializes the motor scale and travel bounds with the driver disabled.
         * @throws std::invalid_argument If the motor scale, bounds or timing are invalid.
         */
        Stepper(StepDirDriver &driver, AxisConfig config)
            : driver_(driver), cfg_(config) {
            const auto positiveFinite = [](double x) { return std::isfinite(x) && x > 0.0; };
            if (!positiveFinite(cfg_.fullStepsPerMotorRev) || !positiveFinite(cfg_.microsteps) ||
                !positiveFinite(cfg_.motorRevsPerOutputRev))
                throw std::invalid_argument("Motor factors must each be finite and positive");
            const double stepsPerRev = cfg_.fullStepsPerMotorRev * cfg_.microsteps *
                                       cfg_.motorRevsPerOutputRev;
            if (!std::isfinite(stepsPerRev) || stepsPerRev <= 0.0 ||
                !std::isfinite(cfg_.minDegrees) || !std::isfinite(cfg_.maxDegrees) ||
                cfg_.minDegrees >= cfg_.maxDegrees || cfg_.pulseHighUs == 0 ||
                cfg_.pulsePeriodUs <= cfg_.pulseHighUs || cfg_.directionSetupUs == 0)
                throw std::invalid_argument("Invalid axis configuration");
            stepsPerDegree_ = stepsPerRev / units::degreesPerRevolution;
            if (!positiveFinite(stepsPerDegree_))
                throw std::invalid_argument("Unrepresentable motor scale");
            minStep_ = checkedIndex(std::ceil(cfg_.minDegrees * stepsPerDegree_));
            maxStep_ = checkedIndex(std::floor(cfg_.maxDegrees * stepsPerDegree_));
            if (minStep_ >= maxStep_) throw std::invalid_argument("Empty axis travel");
            driver_.step(false);
            driver_.enable(false);
        }

        /**
         * Records the result of external driver and fault checks.
         * Marking the driver unavailable disables it and clears the home reference.
         */
        void setDriverReady(bool ready) {
            driverReady_ = ready;
            if (!ready) {
                homed_ = false;
                high_ = false;
                driver_.step(false);
                driver_.enable(false);
            }
        }

        /**
         * Records the measured result of a separate physical homing routine.
         * This function does not move the motor to find its reference position.
         * @return True if the reference was accepted and the driver enabled.
         */
        bool markHomed(double measuredDegrees) {
            if (!driverReady_ || !inRange(measuredDegrees) || high_) return false;
            position_ = target_ = quantize(measuredDegrees);
            homed_ = true;
            directionKnown_ = false;
            nextRiseUs_ = 0;
            driver_.enable(true);
            return true;
        }

        /**
         * @return True if the driver is ready, the axis is homed and the target
         * is within the configured mechanical bounds.
         */
        bool canMove(double degrees) const {
            return homed_ && driverReady_ && inRange(degrees);
        }

        /**
         * Sets a target angle rounded to the nearest allowed microstep.
         * @return True if the target was accepted, false otherwise.
         */
        bool move(double degrees) {
            if (!canMove(degrees)) return false;
            target_ = quantize(degrees);
            return true;
        }

        /**
         * Stops new step pulses while maintaining the driver enable state.
         */
        void hold() {
            target_ = position_;
        }

        /**
         * @return The estimated angle in degrees based on commanded steps.
         */
        double degrees() const {
            return position_ / stepsPerDegree_;
        }

        /**
         * @return True if the target step count is reached and the STEP output is low.
         */
        bool settled() const {
            return position_ == target_ && !high_;
        }

        /**
         * @return True if a home reference is recorded and has not been invalidated.
         */
        bool homed() const {
            return homed_;
        }

        /**
         * Advances at most one output edge using local monotonic microseconds.
         * Call repeatedly from the event loop or timer. Late calls slow movement
         * instead of sending a burst of overdue pulses.
         */
        void tick(std::uint64_t nowUs) {
            if (high_) {
                if (nowUs >= lowAtUs_) {
                    driver_.step(false);
                    high_ = false;
                    nextRiseUs_ = std::max(nextRiseUs_, nowUs + (cfg_.pulsePeriodUs - cfg_.pulseHighUs));
                }
                return;
            }
            if (!homed_ || !driverReady_ || position_ == target_) return;
            const bool positive = target_ > position_;
            if (!directionKnown_ || positive != positive_) {
                positive_ = positive;
                directionKnown_ = true;
                driver_.direction(positive != cfg_.invertDirection);
                directionReadyUs_ = nowUs + cfg_.directionSetupUs;
                return;
            }
            if (nowUs < directionReadyUs_ || nowUs < nextRiseUs_) return;
            driver_.step(true);
            high_ = true;
            position_ += positive ? 1 : -1; // Commanded position, not encoder feedback.
            lowAtUs_ = nowUs + cfg_.pulseHighUs;
            nextRiseUs_ = nowUs + cfg_.pulsePeriodUs;
        }

    private:
        /**
         * Converts an integral-valued double to a signed step index after checking bounds.
         * INT64_MAX can round up to 2^63 as a double, so the upper bound is exclusive.
         */
        static std::int64_t checkedIndex(double integral) {
            const double lower = static_cast<double>(std::numeric_limits<std::int64_t>::min());
            const double upperExclusive = -lower;
            if (!std::isfinite(integral) || integral < lower || integral >= upperExclusive)
                throw std::invalid_argument("Step index exceeds int64 range");
            return static_cast<std::int64_t>(integral);
        }

        /**
         * @return True if the angle is finite and within the configured travel bounds.
         */
        bool inRange(double angle) const {
            return std::isfinite(angle) && angle >= cfg_.minDegrees && angle <= cfg_.maxDegrees;
        }

        /**
         * Rounds an angle to a representable microstep within the travel bounds.
         */
        std::int64_t quantize(double angle) const {
            return std::clamp(checkedIndex(std::round(angle * stepsPerDegree_)), minStep_, maxStep_);
        }
        StepDirDriver &driver_;
        AxisConfig cfg_;
        double stepsPerDegree_ = 0;
        /* Step bounds, commanded position and target, without encoder feedback. */
        std::int64_t minStep_ = 0, maxStep_ = 0, position_ = 0, target_ = 0;

        /* Local deadlines for pulse completion, the next pulse and direction setup. */
        std::uint64_t lowAtUs_ = 0, nextRiseUs_ = 0, directionReadyUs_ = 0;
        bool driverReady_ = false, homed_ = false, high_ = false;
        bool directionKnown_ = false, positive_ = true;
    };
} // namespace rats

#endif // RATS_STEPPER_HPP
