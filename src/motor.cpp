/**
 * @file motor.cpp
 *
 * @brief GPIO stepper drivers and the dedicated antenna control loop.
 */

#include "motor.hpp"
#include "constants.hpp"
#include "hardware/gpio.h"
#include "mover.hpp"
#include "pico/critical_section.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pins.hpp"
#include <array>
#include <cmath>
#include <stdexcept>

namespace {
    namespace tracking = constants::tracking;

    struct MotorCommand {
        rats::TrackingTarget target {};
        std::uint64_t generation = 0;
    };

    // Core 0 writes the station before launching core 1, then leaves it fixed.
    rats::StationReference stationReference {};
    MotorCommand latestCommand;
    critical_section_t commandLock;
    alignas(tracking::motorStackAlignmentBytes) std::uint32_t motorStack[tracking::motorStackWords];
    bool prepared = false;
    bool started = false; // Accessed only by the core 0 API.

    class GpioDriver : public rats::StepDirDriver {
    public:
        explicit GpioDriver(pins::MotorPins motorPins) : pins_(motorPins) {
            // Set output latches before enabling output mode.
            for (int pin : {pins_.step, pins_.direction, pins_.enable})
                gpio_init(pin);
            gpio_put(pins_.step, false);
            gpio_put(pins_.direction, false);
            gpio_put(pins_.enable, pins_.enableActiveLow);
            for (int pin : {pins_.step, pins_.direction, pins_.enable})
                gpio_set_dir(pin, GPIO_OUT);
            gpio_init(pins_.fault);
            gpio_set_dir(pins_.fault, GPIO_IN);
            if (pins_.faultActiveLow)
                gpio_pull_up(pins_.fault);
            else
                gpio_pull_down(pins_.fault);
        }

        void enable(bool on) override {
            gpio_put(pins_.enable, on != pins_.enableActiveLow);
        }

        void direction(bool positive) override {
            gpio_put(pins_.direction, positive);
        }

        void step(bool high) override {
            gpio_put(pins_.step, high);
        }

        bool healthy() const {
            return gpio_get(pins_.fault) == pins_.faultActiveLow;
        }

    private:
        pins::MotorPins pins_;
    };

    /**
     * Validates configuration without touching hardware. Existing radio and SD
     * interfaces retain their pins; GP23/24/25 are reserved by the Pico board.
     */
    bool validConfiguration() {
        if (!tracking::configurationVerified ||
            !std::isfinite(tracking::rocketUnitsPerDegree) || tracking::rocketUnitsPerDegree <= 0 ||
            !std::isfinite(tracking::rocketAltitudeMetresPerUnit) || tracking::rocketAltitudeMetresPerUnit <= 0 ||
            !std::isfinite(tracking::rocketAltitudeToMslOffsetM) ||
            !std::isfinite(tracking::stationAntennaToPivotM) ||
            !std::isfinite(tracking::azimuthAtZeroDeg) || tracking::rocketFixValidMask == 0 ||
            (tracking::rocketFixValidValue & tracking::rocketFixValidMask) != tracking::rocketFixValidValue ||
            tracking::staleAfterUs == 0 || tracking::motorMailboxIntervalUs == 0)
            return false;

        const std::array assigned {
            pins::gnssSda, pins::gnssScl, pins::arm, pins::reference,
            pins::azimuth.step, pins::azimuth.direction, pins::azimuth.enable, pins::azimuth.fault,
            pins::elevation.step, pins::elevation.direction, pins::elevation.enable, pins::elevation.fault};
        const std::array reserved {RFM_TX, RFM_RX, SPI_MISO, SPI_MOSI, SPI_SCK, SPI_CS,
                                  pins::board::smpsMode, pins::board::vbusSense, pins::board::led};
        for (std::size_t i = 0; i < assigned.size(); ++i) {
            if (assigned[i] < 0 || assigned[i] > pins::board::lastAvailableGpio)
                return false;
            for (int pin : reserved)
                if (assigned[i] == pin)
                    return false;
            for (std::size_t j = 0; j < i; ++j)
                if (assigned[i] == assigned[j])
                    return false;
        }
        // Check that both pins expose the selected I2C controller's functions.
        if (pins::gnssSda % pins::board::i2cPinFunctionPeriod != pins::board::i2c0SdaOffset ||
            pins::gnssScl % pins::board::i2cPinFunctionPeriod != pins::board::i2c0SclOffset)
            return false;

        struct NoOutput : rats::StepDirDriver {
            void enable(bool) override {}
            void direction(bool) override {}
            void step(bool) override {}
        } driver;
        try {
            rats::Stepper azimuth(driver, tracking::azimuthConfig);
            rats::Stepper elevation(driver, tracking::elevationConfig);
            azimuth.setDriverReady(true);
            elevation.setDriverReady(true);
            return azimuth.markHomed(tracking::azimuthReferenceDeg) &&
                   elevation.markHomed(tracking::elevationReferenceDeg);
        } catch (const std::invalid_argument &) {
            return false;
        }
    }

    /**
     * Runs only on core 1. GNSS reads, SD writes and USB servicing stay on core 0.
     * Software pulse timing must be measured on the installed Pico.
     */
    void motorLoop() {
        GpioDriver azimuthDriver(pins::azimuth), elevationDriver(pins::elevation);
        rats::Stepper azimuth(azimuthDriver, tracking::azimuthConfig);
        rats::Stepper elevation(elevationDriver, tracking::elevationConfig);
        rats::Mover mover(azimuth, elevation, stationReference.position,
                          tracking::azimuthAtZeroDeg, tracking::staleAfterUs);
        bool referenceWasPressed = !gpio_get(pins::reference);
        MotorCommand command;
        std::uint64_t nextMailboxUs = 0;
        std::uint64_t processedGeneration = 0;
        while (true) {
            const auto nowUs = time_us_64();
            const bool armed = !gpio_get(pins::arm);
            const bool referencePressed = !gpio_get(pins::reference);
            if (!armed || !azimuthDriver.healthy() || !elevationDriver.healthy()) {
                azimuth.setDriverReady(false);
                elevation.setDriverReady(false);
            } else if (!azimuth.homed() && !elevation.homed() &&
                       referencePressed && !referenceWasPressed) {
                // The operator first aligns both axes at their measured references.
                azimuth.setDriverReady(true);
                elevation.setDriverReady(true);
                if (!azimuth.markHomed(tracking::azimuthReferenceDeg) ||
                    !elevation.markHomed(tracking::elevationReferenceDeg)) {
                    azimuth.setDriverReady(false);
                    elevation.setDriverReady(false);
                }
                processedGeneration = 0;
            }
            referenceWasPressed = referencePressed;
            if (nowUs >= nextMailboxUs) {
                critical_section_enter_blocking(&commandLock);
                command = latestCommand;
                critical_section_exit(&commandLock);
                nextMailboxUs = nowUs + tracking::motorMailboxIntervalUs;
            }
            if (command.generation != processedGeneration) {
                auto fix = command.target.fix;
                fix.fixValid = fix.fixValid && nowUs < command.target.expiresAtUs;
                mover.move(fix, nowUs);
                processedGeneration = command.generation;
            }
            if (!command.target.fix.fixValid || nowUs >= command.target.expiresAtUs) {
                // Expire even when core 0 is blocked or no longer publishing.
                azimuth.hold();
                elevation.hold();
            }
            mover.tick(time_us_64());
            tight_loop_contents();
        }
    }
} // namespace

namespace rats::motor {

    bool prepare() {
        if (prepared)
            return true;
        if (!validConfiguration())
            return false;
        for (int pin : {pins::arm, pins::reference}) {
            gpio_init(pin);
            gpio_set_dir(pin, GPIO_IN);
            gpio_pull_up(pin);
        }
        // Keep both driver enables inactive throughout GNSS acquisition.
        for (const auto motorPins : {pins::azimuth, pins::elevation}) {
            gpio_init(motorPins.enable);
            gpio_put(motorPins.enable, motorPins.enableActiveLow);
            gpio_set_dir(motorPins.enable, GPIO_OUT);
        }
        prepared = true;
        return true;
    }

    void start(const StationReference &station) {
        if (!prepared || started || !rats::valid(station.position))
            return;
        stationReference = station;
        latestCommand = {};
        critical_section_init(&commandLock);
        multicore_launch_core1_with_stack(motorLoop, motorStack, sizeof(motorStack));
        started = true;
    }

    void update(const Telemetry &packet, std::uint64_t nowUs) {
        if (!started)
            return;
        const auto target = trackingFromTelemetry(packet, stationReference, nowUs);
        critical_section_enter_blocking(&commandLock);
        latestCommand.target = target;
        ++latestCommand.generation;
        critical_section_exit(&commandLock);
    }

} // namespace rats::motor
