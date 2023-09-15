#ifndef OLED_H
#define OLED_H

#define SSD1306_I2C_ADDRESS 0x3C

#include <Adafruit_SSD1306.h>
#include <Wire.h>

class OLED
{

private:
    Adafruit_SSD1306 display;
public:
    OLED(int resetPin);
    void init();
    void clearDisplay();
    void print(const char* message);
};

#endif