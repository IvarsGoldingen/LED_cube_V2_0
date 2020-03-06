/*********************************************************************************
 * Cube.h
 * Library for controlling a 4x4x4 LED cube with 2 shift registers
 *  
 *  LED numbers
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
//row size of the cube
#define CUBE_SIZE 4
//number of LEDs in a plane of the cube
#define PLANE_SIZE 16

//time each plane is displayed in us; 3333 = 100 Hz refresh
#define PLANETIME 3333
//Short plane time, used for dimming efect in gradualSnake function
#define PLANETIME_SHORT 250

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



class Cube{
  private:
    //Class member variables

    //A pointer to a shift register object
    ShiftRegister* sr1;
    ShiftRegister* sr2;

    //Pins for controlling the shift register
    byte ser1P;
    byte rclk1P;
    byte srclk1P;
    byte ser2P;
    byte rclk2P;
    byte srclk2P;

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
    int patternIdx;

    //Keeps track if next animation should be read when dimming effects are used.
    //Used in gradualSnake animation
    bool nextAnimation = false;
    //Used to dim the displayed leds
    byte dimLevel = 0;
    //Determines how fast the cube is dimmed in sound level modes.
    //Valid values from 1 -50;
    //TODO: make this a setting
    byte dimSpeed = 10;
    //Sound level to be displayed at the led cube 0 = silence, 4 = max sound level
    byte soundLevel = 0;
    //Determines if the shift registers need to be written to in sound mode 2
    byte sndLvl2Status = SM2_NONE;

    //Takes an int where each bit represents a LED and loads it in the shift registers
    void loadPlane(int);

    //turns on the next plane and off the previos on the led cube
    void previousPlaneOff(byte planeNumber);

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


  public:
    /*********************************************************************************
     * A constructor for the class
     * planePins[4] = pins for controlling the 4 horizontal planes of the cube
     * ser1, rclk1, srclk1, ser2, rclk2, srclk2 = Shift register control pins to control
     * the vertical lines of the cube
    *********************************************************************************/
    Cube(byte planePins[4], byte ser1, byte rclk1, byte srclk1, byte ser2, byte rclk2, byte srclk2);

    //Setup function of the cube
    void setup();
    //Loop function of the cube
    void loop();
    //Function for setting sound level for visualizer animations
    void setSoundLevel(byte receivedSoundLvl);
};
#endif