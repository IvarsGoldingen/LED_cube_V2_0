//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"
#include "ShiftRegister.h"

//Constuctor. Get which pins are used for controlling the SR.
ShiftRegister::ShiftRegister(byte serP, byte rclkP, byte srclkP){
    ser = serP;
    rclk = rclkP;
    srclk = srclkP;
}

//Set the control pins as outputs
void ShiftRegister::setup(){
    pinMode(ser, OUTPUT);
    pinMode(rclk, OUTPUT);
    pinMode(srclk, OUTPUT);
}

void ShiftRegister::setOuputs(byte outputBits){
    //will hold the current LEDS supposed value
    bool tempLed = 0;

    //the pins of the shift registers are swithced accordingly to load the ouputs
    digitalWrite(rclk, LOW);
    for (int i = 7; i >=  0; i--){
        //Go through the bits of the received byte
        digitalWrite(srclk, LOW);
        tempLed = outputBits & (1 << i);
        digitalWrite(ser, tempLed);
        digitalWrite(srclk, HIGH);
    }
    digitalWrite(rclk, HIGH);
}