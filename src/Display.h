#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include <Arduino.h>

class Display {
public:
    static const int NUMFLAKES = 15;
    static const int LOGO_WIDTH = 16;
    static const int LOGO_HEIGHT = 16;

    Display(Adafruit_SSD1306& displayRef);

    void init();
    void animate();  // Main method to animate the display

private:
    Adafruit_SSD1306& oledDisplay;
    uint32_t prevLED;
    bool prevLEDStatus;
    float icons[NUMFLAKES][3];

    static const unsigned char PROGMEM logo_bmp[];  // Logo bitmap

    void toggleLED();                   // Toggle the LED at regular intervals
    void drawSnowflakes();              // Draw the snowflakes on the display
    void updateSnowflakePositions();    // Update the position of the snowflakes
};

#endif  // DISPLAY_H
