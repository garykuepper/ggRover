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
    void showTimeSinceStart();

private:
    Adafruit_SSD1306& oledDisplay;
    uint32_t prevLED;
    bool prevLEDStatus;
    float icons[NUMFLAKES][3];

    uint32_t lastTimeUpdate;  // Timestamp for the last time update
    bool isSecondPassed();  // Check if a second has passed since the last update
    void getElapsedTime(uint32_t& hours, uint32_t& minutes, uint32_t& seconds); // Compute the elapsed time
    void renderTime(uint32_t hours, uint32_t minutes, uint32_t seconds); // Display the time on the OLED

    static const unsigned char PROGMEM logo_bmp[];  // Logo bitmap

    void toggleLED();                   // Toggle the LED at regular intervals
    void drawSnowflakes();              // Draw the snowflakes on the display
    void updateSnowflakePositions();    // Update the position of the snowflakes
};

#endif  // DISPLAY_H
