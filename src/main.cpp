#include <Arduino.h>
#include "Display.h"
#include "Blink.h"
#include "Light.h"

Display display;
Blink onboardLED(PC13);
Light led(PA8);

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
