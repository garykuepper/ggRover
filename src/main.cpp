#include <Arduino.h>
//#include "RoverRun.h"
#include "OLEDDriver.h"
#include "BlinkDriver.h"

//RoverRun rover;
//OledDriver oled;
BlinkDriver ledDriver(PC13);
OLEDDriver oledDriver;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //rover.run();
  ledDriver.run();
  ledDriver.printStatusChange();
  

}

