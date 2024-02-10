
/*
 Rotational Antenna Tracking System Software
 Cornell Rocketry Team

 This program receives rocket telemetry, 
 aims the antenna at the rocket's location, 
 and passes the data on to the ground station laptop

 Created SP23 
 by Zach Garcia

 */

// Uncomment if you want debug information about signal strength and motor movements
#define DEBUG

// #include <Stepper.h>
// #include <Servo.h>
// #include <ArduinoJson.h>
#include <cstring>
#include <string>
#include <stdio.h>
#include <iostream>
#include <RadioLib.h>
#include "pico/stdlib.h"
#include "../../lib/RS-FEC.h"
#include "../../src/rfm/pico_hal.h"
#include "../../src/pins.hpp"
#include "stepper.h"
#include "../../lib/pico-servo/include/pico_servo.h"

#include "../../lib/pico-stepper/lib/stepper.c"
#include "../../lib/pico-servo/src/pico_servo.c"


bool launched = false;
bool recPacket = false;
long launchTime = 0;

// initialize the stepper library on pins 7 through 10:
stepper_t stepper;
const uint8_t stepper_pin_1A = 7;
const uint8_t stepper_pin_1B = 8;
const uint8_t stepper_pin_2A = 9;
const uint8_t stepper_pin_2B = 10;
const uint16_t stepper_steps_per_revolution = 200;
const uint16_t platformStepsPerRev = 1320;
const stepper_mode_t stepping_mode = single;
uint8_t speed = 60;

int main() {
    stdio_init_all();

    // Initialize servo
    servo_init();
    servo_clock_auto();
    // Set duty cycle bounds in us
    servo_set_bounds(500, 2500);
    // Attach to pin 12
    servo_attach(12);
    // Set servo to 0 degrees
    servo_move_to(12, 0);

    //Initialize stepper
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B,
                 stepper_pin_2A, stepper_pin_2B,
                 stepper_steps_per_revolution, stepping_mode);
    stepper_set_speed_rpm(&stepper, speed);

    gpio_init(RX_CS);
    gpio_set_dir(RX_CS, GPIO_OUT);
    gpio_put(RX_CS, 1);

    while (true) {
        printf("Hello World");
        // Rotate stepper 3/4 turn.
        stepper_rotate_degrees(&stepper, 270);

        // Rotate servo 1/4 turn.
        printf("quarter turn");
        servo_move_to(12, 90);
        sleep_ms(500);

        // Now rotate these many steps in the oposite direction
        stepper_rotate_steps(&stepper, -45);

        // Return servo to 0
        printf("back to 0");
        servo_move_to(12, 0);
        sleep_ms(500);

        // Increase the speed and rotate 360 degrees
        speed = 50;
        stepper_set_speed_rpm(&stepper, speed);
        stepper_rotate_degrees(&stepper, 360);

        // Rotate servo 180 degrees
        printf("half turn");
        servo_move_to(12, 180);

        // Release the coils and sleep for a while. You can check that
        // the coils are not energised by moving the rotor manually:
        // there should be little resistance.
        stepper_release(&stepper);

        // Return servo to 0 degrees
        printf("back to zero");
        servo_move_to(12, 0);
        sleep_ms(4000);

        // Decrease the speed
        speed = 15;
        stepper_set_speed_rpm(&stepper, speed);
    }
    return 0;
}
