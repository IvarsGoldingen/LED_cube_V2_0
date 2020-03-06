//This needs to be included in the library so it can access standard arduino functions
#include "Arduino.h"
#include "Cube.h"
//Hold the pattern for static pattern display
#include "Big_pattern.h"

Cube::Cube(byte planePins[CUBE_SIZE], byte ser1, byte rclk1, byte srclk1, byte ser2, byte rclk2, byte srclk2){
    ser1P = ser1;
    rclk1P = rclk1;
    srclk1P = srclk1;
    ser2P = ser2;
    rclk2P = rclk2;
    srclk2P = srclk2;

    for (int x = 0; x < CUBE_SIZE; x++){
        planeP[x] = planePins[x];
    }
}

void Cube::setup(){
    //Initialize 2 new shift register objects
    sr1 = new ShiftRegister(ser1P,rclk1P,srclk1P);
    sr2 = new ShiftRegister(ser2P,rclk2P,srclk2P);
    //Execute setup functions for both SR
    //the -> syntax must be used because sr objects are pointers
    sr1->setup();
    sr2->setup();

    //set up plane pins as outputs (set LOW to turn plane ON)
    for (int x = 0; x < CUBE_SIZE; x++){
        pinMode( planeP[x], OUTPUT );
        //all planes on in the beginning
        digitalWrite(planeP[x], HIGH);
    }

    //TODO: plane must be loaded with 255 when soundLevel function is called
    loadPlane(0xffff);
    
}

void Cube::loop(){
    //displayStaticPattern();
    //displayRandom();
    //displaySnake();
    //displayRandomSnakeGradual();
    //call loadPlane(0xff) before turning on this mode
    //displaySoundLevel1();
    //make sndLvl2Status = NONE before turning on this mode
    displaySoundLevel2();
}

//called to display a static pattern on the LED cube
void Cube::displayStaticPattern(){
    //set the beginning of the big pattern
    patternIdx = 0;
    //Static pattern variables
    //Saves LED pattern for a plane from the big pattern
    byte PatternBuf[PLANE_SIZE]; 
    //Saves how long each plane should be displayed from the patter
    byte DisplayTime; // time*100ms to display pattern
    //For controlling when the pattern should be stopeed displaying
    unsigned long EndTime;
    byte plane; // loop counter for cube refresh
    int patBufCntr; // indexes which byte from pattern buffer to use
    int ledRow; // counts LEDs in refresh loop
    int ledCol; // counts LEDs in refresh loop

    // loop over entries in pattern table. End of table is 0 for displayTime
    do {
        //read pattern from PROGMEM and save in array
        memcpy_P( PatternBuf, PatternTable + patternIdx, PLANE_SIZE );
        //Chenge the address for the next plane read from the big pattern
        patternIdx += PLANE_SIZE;
        //read DisplayTime from PROGMEM and increment index
        DisplayTime = pgm_read_byte_near( PatternTable + patternIdx++ );
        //compute EndTime from current time (ms) and DisplayTime
        EndTime = millis() + (((unsigned long) DisplayTime) * TIMECONST);
        //loop while current time < EndTime
        while ( millis() < EndTime )
        {
            //Reset the buffer counter
            patBufCntr = 0; 
            //Loop over planes
            for (plane = 0; plane < CUBE_SIZE; plane++){
                //variable which will be sent to the readLedInt function
                int planeLedBits = 0;
                //bit counter for the planeLedBits variabe
                byte bitCounter = 0;
                //Go through all the LEDs in a plane
                //Go through rows
                for (ledRow = 0; ledRow < CUBE_SIZE; ledRow++){
                    //Go through columns
                    for (ledCol = 0; ledCol < CUBE_SIZE; ledCol++){
                        //Read the patter from the Big pattern and save it in the planeLedBits variable
                        if ((PatternBuf[patBufCntr] & (1 << ledCol)) > 0){
                            //set that bit high in the planeLedBits high too
                            planeLedBits = planeLedBits | 1 << bitCounter;
                        }
                        bitCounter++;
                    }
                    //a byte in the pattern buffer hold value for 4 LEDs in a plane
                    patBufCntr++;
                }
                previousPlaneOff(plane);
                loadPlane(planeLedBits);
                //turn current plane on
                digitalWrite( planeP[plane], HIGH );
                delayMicroseconds( PLANETIME );
            } // for plane
        } // while <EndTime
    } while (DisplayTime > 0); // read patterns until time=0 which signals end
}

void Cube::loadPlane(int leds){
    //Setting a pin LOW will turn it on, so the leds int must be inverted
    leds = ~leds;
    //Load the 8 most significant bits to a temporary byte for setting on a single SR
    byte tempBits = byte(leds >> 8);
    //Set the outputs on SR1
    sr1->setOuputs(tempBits);
    //Load the 8 least significant bits to a temporary byte for setting on a single SR
    tempBits = byte(leds);
    //Set the outputs on SR1
    sr2->setOuputs(tempBits);
}

//TODO: add posibilty of setting how often a random led is generated
void Cube::displayRandom(){
  // get a random number from 0 to 15 and 0 to 3
  // This represents the LED which must be inverted
  int randLed = random(16);
  int randPlane = random(4);
  //<< shifts the 1 in the correct place, then ^ inverses this bit in the array
  ledStates[randPlane] = ledStates[randPlane] ^ (0x1 << randLed);

  for (byte planeCounter = 0; planeCounter < 4; planeCounter++) {
    //turn off the last plane
    previousPlaneOff(planeCounter);
    //write the correct random plane
    loadPlane(ledStates[planeCounter]);
    //turn current plane on
    digitalWrite( planeP[planeCounter], HIGH );
    delayMicroseconds( PLANETIME );
  }
}

void Cube::displaySnake() {
    if (millis() - lastUpdate > snakeSpeed) {
        //New scene must be loaded for the snake animation
        lastUpdate = millis();
        //updates the snake array
        updateSnakeArray();
    }
    for (int planeCounter = 0; planeCounter < 4; planeCounter++) {
    //loop through planes displaying them and delay after setting each one up
        int planeLedBits = 0;
        //initialize the planeLedBits with all the leds in current plane from the snake array
        for (int y = 0; y < 4; y++) {
            //Go through the snake array and chef if there is a point in this plane
            if (planeCounter == snake[y][0]) {
                //Turn that LED bit on
                planeLedBits = planeLedBits | (0x1 << snake[y][1]);
            }
        }
        previousPlaneOff(planeCounter);
        loadPlane(planeLedBits);
        //turn current plane on
        digitalWrite( planeP[planeCounter], HIGH );
        delayMicroseconds( PLANETIME );
    }
}

/**
 * Displays a fading snake
*/
void Cube::displayRandomSnakeGradual() {
    if (nextAnimation) {
        nextAnimation = false;
        //save the tales(last points) value fo dimming effect
        snake[4][0] = snake[3][0];
        snake[4][1] = snake[3][1];
        //updates the snake array
        updateSnakeArray();
        //set the dim to 0 for gradual move
        dimLevel = 0;
    }
    int planeLedBits = 0;
    for (byte dimCounter = 0; dimCounter < 10; dimCounter++){
        for (byte planeCounter = 0; planeCounter < 4; planeCounter++) {
            //if current plane cotains the head of the snake
            if (planeCounter == snake[0][0]) {
                //dim the snakes head up
                if (dimLevel > dimCounter) {
                    planeLedBits = planeLedBits | (0x1 << snake[0][1]);
                }
            }
            //if current plane cotains the previos tail of the snake
            if (planeCounter == snake[4][0]) {
                //dim the snakes tail down
                if (dimLevel <= dimCounter) {
                    planeLedBits = planeLedBits | (0x1 << snake[4][1]);
                }
            }
            //initialize the led int with all the leds in current plane
            for (byte y = 1; y < 4; y++) {
                //if the current point has a led in this plane
                if (planeCounter == snake[y][0]) {
                    planeLedBits = planeLedBits | (0x1 << snake[y][1]);
                }
            }
            previousPlaneOff(planeCounter);
            loadPlane(planeLedBits);
            digitalWrite( planeP[planeCounter], HIGH );
            planeLedBits = 0;
            delayMicroseconds(PLANETIME_SHORT);
        }
    }
    dimLevel += 1;
    if (dimLevel > 10) {
        nextAnimation = true;
    }
}

void Cube::previousPlaneOff(byte planeNumber){
    //turn off the last plane
    if (planeNumber == 0) {
      digitalWrite( planeP[CUBE_SIZE - 1], LOW );
    } else {
      digitalWrite( planeP[planeNumber - 1], LOW );
    }
}

void Cube::updateSnakeArray(){
    //The plane to be turned on
    byte newPlane = 0;
    //The led in that plane to be turned on
    byte newLed = 0;
    //holds the randomly generated snake move
    unsigned char snakeMove = 0;
    bool invalidValue;
    do {
        //Randomly generate a move until a valid one is found
        invalidValue = false;
        //use the random function to decide where the snake is going  
        snakeMove = random(6);
        switch (snakeMove) {
            case MOVE_UP:
                //move one plane up
                newPlane = snake[0][0] + 1;
                //save the current LED position in the newLed variable since this value will not be changed
                newLed =  snake[0][1];
                if (newPlane > 3) {
                    //Out of bounds, generate a random move again
                    invalidValue = true;
                }
                break;
            case MOVE_DOWN:
                //move one plane up
                newPlane = snake[0][0] - 1;
                //save the current LED position in the newLed variable since this value will not be changed
                newLed =  snake[0][1];
                if (newPlane > CUBE_SIZE -1) {
                    //Out of bounds, generate a random move again
                    //since newPlane is a byte it will overfow whn 0 - 1
                    invalidValue = true;
                }
                break;
            case MOVE_LEFT:
                newPlane = snake[0][0];
                newLed = snake[0][1] + 1;
                if (newLed == 4 || newLed == 8 || newLed == 12 || newLed == 16) {
                    //if the head of the sanke is already at the left side of the cube
                    //generate random move again
                    invalidValue = true;
                }
                break;
            case MOVE_RIGHT:
            newPlane = snake[0][0];
            newLed = snake[0][1] - 1;
            if (newLed == 255 || newLed == 3 || newLed == 7 || newLed == 11) {
                //if the head of the sanke is already at the right side of the cube
                //generate random move again
                invalidValue = true;
            }
            break;
            case MOVE_AWAY:
            newPlane = snake[0][0];
            newLed = snake[0][1] + 4;
            if (newLed > (PLANE_SIZE -1)) {
                //Out of bounds, generate new random move
                invalidValue = true;
            }
            break;
            case MOVE_CLOSER:
            newPlane = snake[0][0];
            newLed = snake[0][1] - 4;
            if (newLed > (PLANE_SIZE -1)) {
                //Out of bounds, generate new random move
                invalidValue = true;
            }
            break;
        }
        //check if snake does not hit itself
        for (int x = 1; x < 4; x++) {
            //Loop through the points of the snake which are not its head
            if (newPlane == snake[x][0] && newLed == snake[x][1]) {
                //If the newly generated point colides with any of the current snake points, generate random move again
                invalidValue = true;
                //exit the for loop as
                break;
            }
        }
        //Generate random move until a valid one is found
    } while (invalidValue);

    //move the body of the snake 1 forward
    for (int x = 3; x > 0; x--) {
        snake[x][0] = snake[x - 1][0];
        snake[x][1] = snake[x - 1][1];
    }
    //save the new position of the head of the snake
    snake[0][0] = newPlane;
    snake[0][1] = newLed;
}

void Cube::setSoundLevel(byte receivedSoundLvl){
    if (receivedSoundLvl >= soundLevel) {
        soundLevel = receivedSoundLvl;
        dimLevel = 255;
    }
}

/**
 * Sets how many planes on the cube are lit depending on the sound level
 * Soun level is in the range 0-4 0=silent 4=max sound
 * 4 - light all planes on the cube
 * 1 - load only the bottom plane on the cube
 * 0 - nothing is lit
 * The sound level is reduced in this same function creating a dimming effect
*/
void Cube::displaySoundLevel1() {
  if (soundLevel > 0) {
    for (byte dimCounter = 0; dimCounter < 255; dimCounter++) {
        //use dimCounter to 255 to set the level of the top plane
        //create a dimming effect
        for (byte planeCounter = 0; planeCounter < 4; planeCounter++) {
            //loop through planes
            if (planeCounter < soundLevel - 1) {
                //If the current plane is below the actual sound level, just turn it on
                digitalWrite( planeP[planeCounter], HIGH );
            } else if (planeCounter == soundLevel - 1) {
                //if this plane equals the sound level
                //turn the plane on or off depending on dimLevel
                if (dimLevel > dimCounter) {
                    digitalWrite( planeP[planeCounter], HIGH );
                } else {
                    digitalWrite( planeP[planeCounter], LOW );
                }
            } else {
                //Else the sound level is below the current plane - turn it off
                digitalWrite( planeP[planeCounter], LOW );
            }
        }
    }
    //Dim the top led
    dimLevel = dimLevel - dimSpeed;
    //if the highest levels has almoust dimmed out, turn it off
    //set the next highest level to max
    if (dimLevel < DIM_THRESHHOLD_SOUND1) {
        //Reducce the sound level
      soundLevel--;
      //Set the DIM to max again
      dimLevel = 255;
    }
  }
}


void Cube::displaySoundLevel2() {
    if (soundLevel > 0) {
        for (byte dimCounter = 0; dimCounter < 255; dimCounter++) {
            //use dimCounter to create a dimming effect
            for (byte planeCounter = 0; planeCounter < 4; planeCounter++) {
                //loop through the planes
                //turn off the last plane
                previousPlaneOff(planeCounter);
                if (soundLevel > 2) {
                    //is sound level is more than 2, all leds must be set on all planes
                    if (sndLvl2Status != SM2_ALL_LEDS) {
                        //turn all LEDS on
                        loadPlane(0xffff);
                        sndLvl2Status = SM2_ALL_LEDS;
                    }
                    if (planeCounter == 0 || planeCounter == 3) {
                        //start with dimming only the top and bottom planes
                        if (dimLevel > dimCounter) {
                            digitalWrite( planeP[planeCounter], HIGH );
                        } else {
                            digitalWrite( planeP[planeCounter], LOW );
                        }
                    } else {
                        //turn the middle planes on allways if the sound level is higher thn 2
                        digitalWrite( planeP[planeCounter], HIGH );
                    }
                } else if (planeCounter == 1 || planeCounter == 2) {
                    //if sound level is 1 or 2, the middle leds on the 2nd and 3rd planes must light up
                    if (sndLvl2Status != SM2_MIDDLE_LEDS) {
                        //turn middle LEDS on
                        loadPlane(0x660);
                        sndLvl2Status = SM2_MIDDLE_LEDS;
                    }
                    if (dimLevel > dimCounter) {
                        digitalWrite( planeP[planeCounter], HIGH );
                    } else {
                        digitalWrite( planeP[planeCounter], LOW );
                    }
                } else {
                    //planes 0 and 3 are always off in lower levels of sound mode 2
                    digitalWrite( planeP[planeCounter], LOW );
                }
                //delayMicroseconds( PLANETIME );
            }
        }
        //
        dimLevel = dimLevel - dimSpeed;
        //if the highest levels has almoust dimmed out, turn it off
        //set the next highest level to max
        if (dimLevel < 50) {
            if (soundLevel > 2) {
                soundLevel = 1;
            } else {
                soundLevel = 0;
            }
            dimLevel = 255;
        }
    }
}
