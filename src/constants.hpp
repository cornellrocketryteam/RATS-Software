#include "stepper.h"

namespace constants {
    // Packet Format
    const uint8_t MSG_LEN = 86;
    const uint8_t ALTITUDE_OFFSET = 9;
    const uint8_t LATITUDE_OFFSET = 13;
    const uint8_t LONGITUDE_OFFSET = 17;

    // Servo
    const uint8_t SERVO_PIN = 12;

    // Stepper
    const uint8_t STEPPER_PIN_1A = 7;
    const uint8_t STEPPER_PIN_1B = 8;
    const uint8_t STEPPER_PIN_2A = 9;
    const uint8_t STEPPER_PIN_2B = 10;
    const uint16_t STEPPER_STEPS_PER_REV = 200;
    const uint16_t PLATFORM_STEPS_PER_REV = 1320;
    const stepper_mode_t STEPPING_MODE = single;
    const uint8_t SPEED = 60;

    // RATS Position
    const float GROUND_ELEV = 0.0;
    const float GROUND_LAT = 0.0;
    const float GROUND_LONG = 0.0;

    // Radio
    const float FREQ = 915;  // MHz
    const float BW = 125;    // kHz
    const uint8_t SF = 7;    // Between 7 and 12
    const uint8_t CR = 8;    // Between 5 and 8. 4/8 coding ration - one redundancy bit for every data bit
    const uint8_t SW = 0x16; // Sync-word (defines network) Default is 0d18
    const int8_t PWR = 20;   // Between 2 and 17 or 20 for max power

} // namespace constants