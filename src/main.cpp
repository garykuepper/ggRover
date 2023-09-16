#include <Arduino.h>
#include "Display.h"
#include "Blink.h"
#include "Light.h"
#include "DS4_I2C_CONTROL.h"
#include "PS4Monitor.h"

Display display;
Blink onboardLED(PC13);
Light led(PA8);

PS4Monitor ps4Monitor(0x29, display);

const int baudRate = 115200;


void setup() {
  Serial.begin(baudRate);
  
  display.init();
  onboardLED.init();
  led.init();
  ps4Monitor.begin();

}

void loop() {
   // display.animate();
  display.showTimeSinceStart();
  onboardLED.update();
  led.update();

  //ps4Monitor.update();
  
}
