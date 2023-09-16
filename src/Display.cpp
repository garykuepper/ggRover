#include "Display.h"
#include <Arduino.h>

#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32

const unsigned char PROGMEM logo_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};

Display::Display() : oledDisplay(OLED_WIDTH, OLED_HEIGHT, &Wire) {}

void Display::init()
{

    // Serial.begin(9600);

    if (!oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR))
    {
        Serial.println(F("SSD1306 allocation failed"));
        while (true)
            ; // Halt execution
    }

    oledDisplay.display();
    oledDisplay.clearDisplay();

    lastTimeUpdate = 0;
}

void Display::animate()
{
    oledDisplay.clearDisplay();
    for (int i = 0; i < NUMFLAKES; i++)
    {
        oledDisplay.drawBitmap(icons[i][0], icons[i][1], logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
    }
    oledDisplay.display();
    updateSnowflakes();
}

void Display::getElapsedTime(uint32_t &hours, uint32_t &minutes, uint32_t &seconds)
{
    uint32_t totalMillis = millis();
    uint32_t totalSeconds = totalMillis / 1000;
    hours = totalSeconds / 3600;
    uint32_t remainingSeconds = totalSeconds % 3600;
    minutes = remainingSeconds / 60;
    seconds = remainingSeconds % 60;
}

void Display::renderTime(uint32_t hours, uint32_t minutes, uint32_t seconds) {
    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(WHITE);
    oledDisplay.setCursor(0, 20);    
    String timeStr = "Uptime: " + padWithZero(hours) + ":" + padWithZero(minutes) + ":" + padWithZero(seconds);
    oledDisplay.print(timeStr);
}

String Display::padWithZero(uint32_t value) {
    if (value < 10) {
        return "0" + String(value);
    }
    return String(value);
}
String Display::padWithSpace(uint32_t value) {
    if (value < 100) {
        if (value <10) {
            return "  " + String(value);
        }
        return " " + String(value);
    }    
    return String(value);
}
void Display::showTimeSinceStart()
{
    uint32_t hours, minutes, seconds;
    getElapsedTime(hours, minutes, seconds);
    renderTime(hours, minutes, seconds);
}

void Display::updateSnowflakes()
{
    for (int i = 0; i < NUMFLAKES; i++)
    {
        icons[i][1] += icons[i][2];
        if (icons[i][1] >= OLED_HEIGHT)
        {
            icons[i][0] = random(1 - LOGO_WIDTH, OLED_WIDTH);
            icons[i][1] = -LOGO_HEIGHT;
            icons[i][2] = random(1, 6);
        }
    }
}

void Display::clear()
{
    oledDisplay.clearDisplay();
}

void Display::update()
{
    oledDisplay.display();
}

void Display::writeText(int x, int y, const String &text)
{
 
    oledDisplay.setCursor(x, y);
    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(WHITE);    
    oledDisplay.print(text);
 
}

void Display::writeX(int x, int y, int value)
{

    oledDisplay.setCursor(x, y);
    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(WHITE);
    String output = "X: " + String(value);
    oledDisplay.print(output.c_str());
  
}
