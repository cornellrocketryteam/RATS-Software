/**
 * @file constants.hpp
 * @author sj728
 *
 * @brief Constants and other definitions
 */
#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

/**
 * Container for hardware RATS software interacts with
 */
namespace constants
{

    // Radio
    constexpr unsigned int RFM_BAUDRATE = 115200;
    // SD
    constexpr unsigned int sd_baudrate = 125 * 1000000 / 6; // baud

    // Stepper

}; // namespace constants

#endif // CONSTANTS_HPP