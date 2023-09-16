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

String padWithSpaces(int value, int totalChars){
    String result = String(value);
    while (result.length() < totalChars) {
        result = " " + result;
    }
    return result;
}

void updateDisplay()
{
    display.clear();
    display.showTimeSinceStart();
    String xJoy = "L_X:" + padWithSpaces(ps4.getLX(),4) + " R_X:" + padWithSpaces(ps4.getRX(),4);
    String yJoy = "L_Y:" + padWithSpaces(ps4.getLY(),4) + " R_Y:" + padWithSpaces(ps4.getRY(),4);
    display.writeText(0,0,xJoy);
    display.writeText(0,10,yJoy);
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
    timer.run();  // runs display and blink
    ps4.update();
    light.update();
}