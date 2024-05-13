#include "mover.hpp"
#include "pico/stdlib.h"
#include "constants.hpp"
#include "data.hpp"
#include "formulas.hpp"
#include "../lib/pico-servo/include/pico_servo.h"

#include <cstdio>

#include "../lib/pico-servo/src/pico_servo.c"
#include "../lib/pico-stepper/lib/stepper.c"

void Mover::init() {
    // Initialize servo
    servo_init();
    servo_clock_auto();
    // Set duty cycle bounds in us
    servo_set_bounds(500, 2500);
    // Attach to pin 12
    servo_attach(constants::SERVO_PIN);
    // Set servo to 0 degrees
    servo_move_to(constants::SERVO_PIN, 0);

    // Initialize stepper
    stepper_init(&stepper, constants::STEPPER_PIN_1A, constants::STEPPER_PIN_1B, constants::STEPPER_PIN_2A, constants::STEPPER_PIN_2B, constants::STEPPER_STEPS_PER_REV, constants::STEPPING_MODE);
    stepper_set_speed_rpm(&stepper, constants::SPEED);
}

void Mover::move(float rockElev, float rockLat, float rockLong) {
    // Switch to backup data if needed
    if (launched && launchTime == 0) {
        launchTime = to_ms_since_boot(get_absolute_time());
    } else if (launched && !recPacket) {
        // switch to backup data
        int dataIndex = (to_ms_since_boot(get_absolute_time()) - launchTime) / 500;
        rockElev = alt_data[dataIndex];
    }


    // convert coordinates to angles
    double dist = distance(rockLat, rockLong, constants::GROUND_LAT, constants::GROUND_LONG);
#ifdef DEBUG
    printf("\nDistance: ");
    printf("%d", dist);
    printf(" km\n");
#endif // DEBUG
    double bear = bearing(rockLat, rockLong, constants::GROUND_LAT, constants::GROUND_LONG);
#ifdef DEBUG
    printf("Bearing: ");
    printf("%d", bear);
    printf(" degrees\n");
#endif // DEBUG
    double asc = ascension(rockElev, constants::GROUND_ELEV, dist);
#ifdef DEBUG
    printf("Ascension: ");
    printf("%d", asc);
    printf(" degrees\n");
#endif // DEBUG
    int target = -1 * (bear / 360) * constants::PLATFORM_STEPS_PER_REV;
    int stepsToTake = target - stepPos;
    if (target - stepPos > (constants::PLATFORM_STEPS_PER_REV / 2)) {
        stepsToTake = (target - stepPos) - constants::PLATFORM_STEPS_PER_REV;
    } else if (target - stepPos < -(constants::PLATFORM_STEPS_PER_REV / 2)) {
        stepsToTake = (target - stepPos) + constants::PLATFORM_STEPS_PER_REV;
    } else {
        stepsToTake = target - stepPos;
    }
    stepPos = target;
    // aim antenna
    servo_move_to(constants::SERVO_PIN, 90 - asc);
    if (stepsToTake != 0) {
        stepper_rotate_steps(&stepper, stepsToTake);
        stepper_release(&stepper);
    }
}