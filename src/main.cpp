#include <Arduino.h>

#include "BlinkDriver.h"


BlinkDriver ledDriver(PC13);


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
  ledDriver.printElapsedTime();

}

