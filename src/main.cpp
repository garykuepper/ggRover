#include <Arduino.h>
//#include "RoverRun.h"
#include "BlinkDriver.h"

//RoverRun rover;
//OledDriver oled;
BlinkDriver ledDriver(PC13);
OLEDDriver oledDriver;

void setup() {
  Serial.begin(9600);

  display.init();
  onboardLED.init();
  led.init();
}

void loop() {
   // display.animate();
    display.showTimeSinceStart();
    onboardLED.update();
    led.update();
}
