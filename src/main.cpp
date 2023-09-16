#include <Arduino.h>
#include "PS4Monitor.h"
#include "Display.h"
#include <SimpleTimer.h>

const uint32_t baudRate = 115200;
const uint16_t dispRefresh = 20;
PS4Monitor ps4;
Display display;
SimpleTimer timer;

void updateDisplay()
{
    display.clear();
    display.writeX(0, 0, ps4.getLX());
    display.writeX(0, 10, ps4.getLY());
    display.showTimeSinceStart();
    display.update();
}

void setup()
{
    // while (!Serial);
    // Serial.begin(baudRate);
    ps4.begin();
    display.init();
    int displayTimer = timer.setInterval(dispRefresh, updateDisplay);
}

void loop()
{
    timer.run();
    ps4.update();
}