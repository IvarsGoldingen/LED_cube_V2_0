#include <Arduino.h>
#include <Cube.h>

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

void getCommandFromSerial();

//Array for passing the plane pins to the Cube object
byte planePins[] = {PLANE_1,PLANE_2,PLANE_3,PLANE_4};

//Initialize the cube object
Cube cube(planePins, SER1, RCLK1, SRCLK1, SER2, RCLK2, SRCLK2);

void setup() {
  //setup serial communication
  Serial.begin(57600);
  Serial.println("CUBE V2.0 START");
  //setup the LED cube
  cube.setup();
  //seed the random function from an unconnected analog read pin
  randomSeed(analogRead(A1));
  //To make the cube dim from max to min when initially turned on
  cube.setSoundLevel(4);
}

void loop() {
  //loop the LED cube
  cube.loop();
  getCommandFromSerial();
}

void getCommandFromSerial() {
  //allways reads the last byte received
  while (Serial.available() > 0) {
    // read the incoming byte:
    byte incomingByte = Serial.read();
    Serial.println(incomingByte);
    if (incomingByte == '1'){
      cube.setSoundLevel(1);
    } else if (incomingByte == '2'){
      cube.setSoundLevel(2);
    } else if (incomingByte == '3'){
      cube.setSoundLevel(3);
    } else if (incomingByte == '4'){
      cube.setSoundLevel(4);
    }
  }
}