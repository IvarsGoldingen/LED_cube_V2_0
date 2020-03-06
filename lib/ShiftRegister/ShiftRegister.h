/*********************************************************************************
 * ShiftRegister.h
 * Library for using a shift register with 8 outputs
 * by Ivars Briedis
 * V1.0 created 2020.03.01
*********************************************************************************/

//This prevents problems if somehow this calls was implemented twice in the same scetch
#ifndef ShiftRegister_h
#define ShiftRegister_h

//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"

class ShiftRegister {
    private:
        //Pins for controlling the shift register
        byte ser;
        byte rclk;
        byte srclk;

    public:
        //constructor. The pins used for controlling the register are passed here.
        ShiftRegister(byte serP, byte rclkP, byte srclkP);

        //Setup function of the ShiftRegister
        void setup();

        //Function for setting the outputs of the shift register
        //outputBits - each bit represents the state of the apropriate output on the SR
        void setOuputs(byte outputBits);
};

#endif