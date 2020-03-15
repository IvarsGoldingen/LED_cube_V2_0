/*********************************************************************************
 Max9841.h
 * Library for for using a microphone Max9841 board
 * * by Ivars Briedis
 * Created 2020.03.12
*********************************************************************************/
//This prevents problems if somehow this calls was implemented twice in the same scetch
#ifndef Max9841_h
#define Max9841_h

//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"

#define GAIN_LOW 0
#define GAIN_MEDIUM 1
#define GAIN_HIGH 2

//Default sound levels
#define SOUND_LEVEL_1 300
#define SOUND_LEVEL_2 450
#define SOUND_LEVEL_3 550
#define SOUND_LEVEL_4 650

class Max9841 {
    private:
    //microphone pin
    byte micP;
    //gain set pin
    byte gainP;

    //get initial sound levels from defaults
    int soundLevel1 = SOUND_LEVEL_1;
    int soundLevel2 = SOUND_LEVEL_2;
    int soundLevel3 = SOUND_LEVEL_3;
    int soundLevel4 = SOUND_LEVEL_4;

    public:
    /***
     * Constuctor
     * micP - analog input pin on which the Max9841 output pin is connected
     * gainP - pin for controlling the gain on the Max9841 chip
     */ 
    Max9841(byte micPin, byte gainPin);
    //setup function. Must be called from the main setup
    void setup();
    /***
     * void setGain(byte gain);
     * Set the gain to either 40, 50 or 60 dB on the Max9841 board
     * 0 - 40 dB
     * 1 - 50 dB
     * 2 - 60 dB
     */
    void setGain(byte gain);
    /***
     * getSoundLevel();
     * returns 0-4 depending on the noise
     * 0 - no sound
     * 4 - max sound
     */
    byte getSoundLevel();
    //set levels for reacting to sound
    //the output from the electric schematic does not give more that 680
    //but the analog input can read from 0-1024
    void setSoundLevels(int lvl1, int lvl2, int lvl3, int lvl4);
};

#endif