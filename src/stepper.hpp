/**
 * @file stepper.hpp
 * @author sj728
 * 
 * @brief Stepper motor interface for the mover namespace
 */ 

#ifndef STEPPER_HPP
#define STEPPER_HPP


/**
 * Class for controlling a stepper motor.
 */
class Stepper {
    
public:
    /* 
    * Initializes the stepper motor
    */
    Stepper(/*Args ...args*/); 

    /*
    * Moves the stepper motor to a new position
    */
    void move(/*Args ...args*/);
};



#endif // STEPPER_HPP