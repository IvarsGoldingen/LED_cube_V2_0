/*********************************************************************************
 * Cube.h
 * Library for controlling a 4x4x4 LED cube with 2 shift registers
 *  
 *  LED numbers when looking at the cobe from the front
 *  [15]  [14]  [13] [12]
 *  [11]  [10]  [9]  [8]
 *  [7]   [6]   [5]  [4]
 *  [3]   [2]   [1]  [0]
 *        Front
 * 
 * * by Ivars Briedis
 * Created 2020.02.23
*********************************************************************************/
//This prevents problems if somehow this calls was implemented twice in the same scetch
#ifndef Cube_h
#define Cube_h

//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"
//Shift register library
#include "ShiftRegister.h"
//Microphone library
#include "Max9841.h"

//row size of the cube
#define CUBE_SIZE 4
//number of LEDs in a plane of the cube
#define PLANE_SIZE 16

//time each plane is displayed in us; 3333 = 100 Hz refresh
#define PLANETIME 3333
//Short plane time, used for dimming efect in gradualSnake function
#define PLANETIME_SHORT 250
//time each plane is displayed in the reeived pattern mode. uS
#define PLANETIME_REC_PAT 500


//multiplies the display time in the big_pater to get ms. Big patter holds
//bytes, that is why ms can't be saved there dirrectly
#define TIMECONST 20 

//***MOVEMENTS FOR SNAKE MODE***
#define MOVE_UP 0 
#define MOVE_DOWN 1 
#define MOVE_LEFT 2 
#define MOVE_RIGHT 3 
#define MOVE_AWAY 4 
#define MOVE_CLOSER 5

//At what level should the current DIM level be turned off for sound Level 1 function
#define DIM_THRESHHOLD_SOUND1 50

//Determines what value is set in the shift registers in sound mode 2
#define SM2_NONE 0 
#define SM2_ALL_LEDS 1 
#define SM2_MIDDLE_LEDS 2 

//fireworks animation steps
#define FW_ST0_INIT 0
#define FW_ST1_UP1 1
#define FW_ST2_EXPLOSION 2
#define FW_ST3_DOWN 3

//The plane at which the fireworks stop falling down
#define FW_LOWEST_FALL_PLANE 0

//fireworks animation falling rockets levels
//for creating a burnout effect
#define FW_FALL_LVL3 255
#define FW_FALL_LVL2 120
#define FW_FALL_LVL1 50
#define FW_FALL_LVL0 20

//Max numbers of FW in the FW animation
#define MAX_NUMBER_OF_FW 5

//MODES
#define OFF_MODE 10
#define STATIC_PATTERN_MODE 11
#define RANDOM_MODE 12
#define SOUND_MODE1 13
#define SOUND_MODE2 14
#define SNAKE_MODE1 15
#define SNAKE_MODE2 16
#define FIREWORKS_MODE 17
#define CUSTOM_PATTERN_MODE 18

//Pattern receive max time between bytes  for timeOut
#define PATTERN_RECEIVE_TO 10000

class Cube{
  private:
    //Class member variables

    //A pointer to a shift register object
    ShiftRegister* sr1;
    ShiftRegister* sr2;

    //A pointer to the microphone object
    Max9841* mic;

    //Pins for controlling the shift register
    byte ser1P;
    byte rclk1P;
    byte srclk1P;
    byte ser2P;
    byte rclk2P;
    byte srclk2P;

    //Pins for controlling the microphone
    byte micOutP;
    byte micGainP;

    //pins for controlling the planes
    byte planeP[CUBE_SIZE];

    //Saves the state of each LED on the cube. Used for random mode.
    //Each int in the array represents a plane on the cube
    unsigned int ledStates [4] = {0,0,0,0};

    //The time when the last update on the CUBE pins was made is saved here
    //Used to determine if go to the next "scene"
    unsigned long lastUpdate = 0;

    //Determines how fast the snake moves through the cube for displaySnake() mode
    //TODO:make this updatable
    unsigned int snakeSpeed = 100;
    byte previousPosition [2] = {99, 99};

    //saves current position of the snake in the led cube. The snake is 4 LEDS long.
    //+1 more for dimming of the tale in case of gradualSnakeMode
    //first value is plane, second the LED in that plane.
    byte snake[5][2] =
    {
      //Initially set the snake horizontally on plane 0
      {0, 0}, //the head of the snake
      {0, 1},
      {0, 2},
      {0, 3}, //tail of the sanke
      {0, 3}
    };

    //the index of the big pattern which should currently be read if the big pattern is being displayed
    int patternIdx = 0;

    //Keeps track if next animation should be read when dimming effects are used.
    //Used in gradualSnake animation
    bool nextAnimation = false;
    //Used to dim the displayed leds
    unsigned char dimLevel = 0;
    //Determines how fast the cube is dimmed in sound level modes.
    //Valid values from 1 -50;
    //TODO: make this a setting
    unsigned char dimSpeed = 15;
    //Sound level to be displayed at the led cube 0 = silence, 4 = max sound level
    byte soundLevel = 0;
    //Determines if the shift registers need to be written to in sound mode 2
    byte sndLvl2Status = SM2_NONE;
    //keeps track in which step the program is in the fireworks animation
    unsigned char fireworksStep = 0;
    //keeps track of which level should currently be dimmed in the fireworks animation
    unsigned char dimPlaneFW = 0;
    //holds the led positions which should be lit when the fireworks animation is displayed
    unsigned int fwExplosionInt = 0;
    //for creating a burnout effect in the fireworks animation
    unsigned char burnoutEffectLvl = FW_FALL_LVL1;
    //Max fade levels for creating a burnout rocket animation for fireworks animation
    unsigned char fwFallDownLevels[4] = {FW_FALL_LVL0, FW_FALL_LVL1,FW_FALL_LVL2,FW_FALL_LVL3};
    //The animation currently being displayed
    byte mode = STATIC_PATTERN_MODE;
    //Scene being filled from serial, but not yet displayed
    unsigned int receivedSceneBuffer[4] = {0, 0, 0, 0};
    //Scene received from serial and displayed
    unsigned int receivedScene[4] = {0, 0, 0, 0};
    //Last time the plane was switched in receivedPatternMode
    unsigned long receivedPatternRefreshTime = 0;

    //debug function
    void printPlaneBits(int x);

    //Takes an int where each bit represents a LED and loads it in the shift registers
    void loadPlane(int);

    //turns on the next plane and off the previos on the led cube
    void previousPlaneOff(byte planeNumber);
    //NOT USED because timer1 is used for PWM
    void setupTimer1Interrupt();
     //Sets up Timer2 interrupt to adjust how often soudnis read
    void setupTimer2Interrupt();

    //***DISPLAY MODES FUNCTIONS FOR THE CUBE***
    //function that displays a static pattern on the led cube
    void displayStaticPattern();
    //LEDs turn on and off randomly
    void displayRandom();
    //A snake displayed on the LED cube
    void displaySnake();
    //Display fading snake
    void displayRandomSnakeGradual();
    //Calculate the next points of the snake. Updates the snake array
    void updateSnakeArray();
    //Displays sound level vertically on the cube.
    void displaySoundLevel1();
    //Displays sound level in 2 levels. High all cube lit, low inner cube lit
    void displaySoundLevel2();
    //TODO:random fireworks shooting up
    void displayFireWorks();
    //Play the scene received from serial
    void displayReceivedPattern();
    /**
     * helper function for fireworks animation.
     * Retuns true if dimming finished
     * plane, the plane which is dimmed down, the above one will be dimmed up
     * */ 
    bool dimUp(unsigned char plane);
        /**
     * helper function for fireworks animation.
     * Retuns true if dimming finished
     * plane, the plane which is dimmed down, the below one will be dimmed up
     * the leds looose brightnessm the lower they fall
     * */ 
    bool dimDown(unsigned char plane);
    //Generates random fw animation for the displayFireWorks() function to play
    void generateRandomFw();
    //will fill the fwExplosionArray with the leds that should be light up for the fireworks animation
    void fillFwExplosionInt(char startingPoint);
    //the analog microphone pin is read certain number of times per second
    //this function check if the timer has passed and reads the sound level from the mic
    //and sets it on the LEDS
    void checkMic();
    //set all planes on or off
    void setAllPlanes(bool);
    //set up dim speed depending on command from serial
    void setDimSpeedFromSerial(byte serialDimSpeed);
    //Write the received scene buffer into the actual displayed scene
    void fillReceivedSceneBuffer();


  public:
    /*********************************************************************************
     * A constructor for the class
     * planePins[4] = pins for controlling the 4 horizontal planes of the cube
     * ser1, rclk1, srclk1, ser2, rclk2, srclk2 = Shift register control pins to control
     * the vertical lines of the cube
     * micOutput, micGain microphone reading and control pins
    *********************************************************************************/
    Cube(byte planePins[4],
      byte ser1, 
      byte rclk1, 
      byte srclk1, 
      byte ser2, 
      byte rclk2, 
      byte srclk2,
      byte micOutput,
      byte micGain);

    
    //Function which will be called from the interrup
    static void Timer2ISR();
    //Setup function of the cube
    void setup();
    //Loop function of the cube
    void loop();
    //Function for setting sound level for visualizer animations
    void setSoundLevel(byte receivedSoundLvl);
    //Switch to the next animation mode
    void nextMode();
    //Sets the next mode
    void setMode(byte);
    //Allows changing of gain for the mic board
    void toggleMicGain();
    //Handle the commands received from serial port in the cube class
    void handleCubeSerialCommands(byte data);
    //When cube starts up
    void set_random_non_sound_mode();
  protected:
    //Bool that will be updated in the ISR so the program reads the analog input
    volatile static bool checkSoundLevel;
};
#endif