/**
 * @file stepper.hpp
 * @author sj728
 * 
 * @brief Interface for controlling motors for RATS
 */ 

#ifndef MOVER_HPP
#define MOVER_HPP

#include "stepper.hpp"


/**
 * Class for controlling the elevator and rotator stepper motors.
 */
class Mover {

public:
    /* 
    * Initializes the elevator and rotator stepper motors
    */
    Mover(/*Args ...args*/);

    /*
    * Moves the stepper motors to a new position
    */
    void move(/*Args ...args*/); 

private:
    /*
    * This stepper is in control of moving the RATS antenna up and down, 
    * from ground level (0 degrees) to ground level on opposide side (180 degrees) 
    */
    Stepper elevator;

    /*
    * This stepper is in control of rotating the RATS antenna left and right, 
    * from 90 degrees to the left to 90 degrees to the right
    */
    Stepper rotator;
};

#endif // MOVER_HPP