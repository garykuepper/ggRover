#include <Arduino.h>
#include "PS4Monitor.h"
#include "Display.h"
#include <SimpleTimer.h>
#include "Blink.h"
#include "Light.h"

const uint32_t baudRate = 115200;
const uint16_t dispRefresh = 20;
const uint16_t blinkRefresh = 1000;
PS4Monitor ps4;
Display display;
SimpleTimer timer;
Light light(PA8);
Blink blink(PC13);
void updateDisplay()
{
    display.clear();
    display.writeX(0, 0, ps4.getLX());
    display.writeX(0, 10, ps4.getLY());
    display.showTimeSinceStart();
    display.update();
}
void blinkToggle()
{
    blink.toggle();
}
void setup()
{
    // while (!Serial);
    // Serial.begin(baudRate);
    ps4.begin();
    display.init();
    blink.init();
    light.init();
    int displayTimer = timer.setInterval(dispRefresh, updateDisplay);
    int blinkTimer = timer.setInterval(blinkRefresh, blinkToggle);

}

void loop()
{
    timer.run();
    ps4.update();
    light.update();
}