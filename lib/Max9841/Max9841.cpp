//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"
#include "Max9841.h"

//constructor
Max9841::Max9841(byte micPin, byte gainPin){
    micP = micPin;
    gainP = gainPin;
}

void Max9841::setup(){
    //setup Max9841 pins
    pinMode (micP,INPUT);
    //set default gain to maximum
    setGain(GAIN_HIGH);
}

void Max9841::setGain(byte gain){
    if (gain == GAIN_HIGH){
        //the gain pin must be floating for 60dB gain
        pinMode(gainP, INPUT);
        currentGain = GAIN_HIGH;
    } else if (gain == GAIN_MEDIUM){
        //the gain pin must be LOW for 50dB gain
        pinMode(gainP, OUTPUT);
        digitalWrite(gainP, LOW);
        currentGain = GAIN_MEDIUM;
    } else if (gain == GAIN_LOW){
        //the gain pin must be HIGH for 40dB gain
        pinMode(gainP, OUTPUT);
        digitalWrite(gainP, HIGH);
        currentGain = GAIN_LOW;
    } else {
        //Unknown gain setting
        Serial.println("Max9841: Unknown gain setting");
        currentGain = GAIN_HIGH;
    }
}

/**
 * TODO: this woulb be much better if this function would read the state of the pin
 * instead of having a variable to keep track of.
*/
byte Max9841::getMicGain(){
    return currentGain;
}

byte Max9841::getSoundLevel(){
    int soundLevel = analogRead(micP);
    if (soundLevel > soundLevel4){
        return 4;
    } else if (soundLevel > soundLevel3){
        return 3;
    } else if (soundLevel > soundLevel2){
        return 2;
    } else if (soundLevel > soundLevel1){
        return 1;
    } else {
        return 0;
    }
}

void Max9841::setSoundLevels(int lvl1, int lvl2, int lvl3, int lvl4){
    /**
     * TODO: make sanity checks for invalid values, level sequence etc.
    */
    soundLevel1 = lvl1;
    soundLevel2 = lvl2;
    soundLevel3 = lvl3;
    soundLevel4 = lvl4;
}

