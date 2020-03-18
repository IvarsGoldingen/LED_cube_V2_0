#include <Arduino.h>
#include <Cube.h>
#include <ButtonIB.h>

//Shift register control pins
#define SER1 5  //pin 14 on the 75HC595
#define RCLK1 7 //pin 12 on the 75HC595
#define SRCLK1 6 //pin 11 on the 75HC595
#define SER2 2   //pin 14 on the 75HC595
#define RCLK2 3  //pin 12 on the 75HC595
#define SRCLK2 4 //pin 11 on the 75HC595
//Plane control pins
#define PLANE_1 13
#define PLANE_2 12
#define PLANE_3 11
#define PLANE_4 10
//Microphone control pins
#define MIC_OUTPUT_P A0
#define MIC_GAIN_P A5
//Button pin
#define BUTTON_P 8
//INDICATION LED BUTTON
#define INDICATION_LED_P 9

void getCommandFromSerial();
void buttonPressed(byte);
void blinkLed(unsigned int timeMs, byte cycles);

int maxSoundLevel = 0;

//Array for passing the plane pins to the Cube object
byte planePins[] = {PLANE_1,PLANE_2,PLANE_3,PLANE_4};

//Initialize the cube object
Cube cube(planePins, 
    SER1, RCLK1, SRCLK1, SER2, RCLK2, SRCLK2,
    MIC_OUTPUT_P, MIC_GAIN_P);

//Button object
ButtonIB btn(BUTTON_P, *buttonPressed);

void setup() {
  //setup serial communication
  Serial.begin(57600);
  Serial.println("CUBE V2.0 START");
  //setup the LED cube
  cube.setup();
  //seed the random function from an unconnected analog read pin
  randomSeed(analogRead(A1));
  //Have the indication light be slightly when the cube is on
  pinMode(INDICATION_LED_P, OUTPUT);
  analogWrite(INDICATION_LED_P,10);
}

void loop() {
  //loop the LED cube
  cube.loop();
  getCommandFromSerial();
  btn.loop();
}

//Receiving of serial port left in main code, but all data handled in the cube
//object.
void getCommandFromSerial() {
  while (Serial.available() > 0) {
    //Read all the incomming data:
    byte incomingByte = Serial.read();
    //Send data to cube object
    cube.handleCubeSerialCommands(incomingByte);
  }
}

//Function called from a button object, when the button is pressed
void buttonPressed(byte pressType){
  if (pressType == ButtonIB::SHORT_PRESS){
    //blink once for a short press
    blinkLed(100, 1);
    cube.nextMode();
  } else if (pressType == ButtonIB::LONG_PRESS){
    blinkLed(100, 2);
    cube.toggleMicGain();
  }
  analogWrite(INDICATION_LED_P,10);
}

void blinkLed(unsigned int timeMs, byte cycles){
  /**
   * TODO: this would be better without the delays
  */
  for (int i = 0; i < cycles; i++){
    //blink twice for a long press
    digitalWrite(INDICATION_LED_P, HIGH);
    delay(timeMs);
    digitalWrite(INDICATION_LED_P, LOW);
    delay(timeMs);
  }
}