#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

#define NUMFLAKES 15
#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16

class Display {
public:
    Display();
    void init();
    void animate();
    void showTimeSinceStart();
    
    // New methods
    void clear();
    void writeText(int x, int y, const char* text);
    void update();
    void writeX(int x, int y, int value, uint32_t interval);

private:
    Adafruit_SSD1306 oledDisplay;
    uint32_t lastTimeUpdate;
    uint32_t lastXUpdateTime;
    float icons[NUMFLAKES][3];

    bool isSecondPassed();
    void getElapsedTime(uint32_t& hours, uint32_t& minutes, uint32_t& seconds);
    void renderTime(uint32_t hours, uint32_t minutes, uint32_t seconds);
    void updateSnowflakes();
};

#endif
