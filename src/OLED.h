#ifndef OLED_H
#define OLED_H

#define SSD1306_I2C_ADDRESS 0x3C
#define OLED_RESET -1 

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class OLED {
public:
    OLED(int resetPin = -1);  // Default to no reset pin if not provided

    // Initialize the OLED display
    void begin();

    // Display elapsed time on the OLED
    void displayTime(unsigned long elapsedTime);

private:
    Adafruit_SSD1306 display;
};

#endif
