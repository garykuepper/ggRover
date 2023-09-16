#include "Display.h"
#include <Arduino.h>

const unsigned char PROGMEM Display::logo_bmp[] =
{ B00000000, B11000000,
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
  B00000000, B00110000 };


Display::Display(Adafruit_SSD1306& displayRef) 
  : oledDisplay(displayRef), prevLED(0), prevLEDStatus(false) {}

void Display::init() {
    lastTimeUpdate = 0;

    for(int f=0; f< NUMFLAKES; f++) {
        icons[f][0]   = random(1 - LOGO_WIDTH, oledDisplay.width());
        icons[f][1]   = -LOGO_HEIGHT;
        icons[f][2]   = random(1, 6);
    }
}

void Display::animate() {
    toggleLED();
    drawSnowflakes();
    updateSnowflakePositions();
}

void Display::toggleLED() {
    uint32_t currentMillis = millis();
    if ((currentMillis - prevLED) > 1000) {
        prevLED = currentMillis;
        prevLEDStatus = !prevLEDStatus;
        digitalWrite(PC13, prevLEDStatus);
    }
}

void Display::drawSnowflakes() {
    static uint32_t lastUpdate = 0; 
    const uint32_t updateInterval = 150; 

    uint32_t currentMillis = millis();

    // Check if it's time to update the snowflakes
    if (currentMillis - lastUpdate >= updateInterval) {
        lastUpdate = currentMillis;  // Save the last update time
        
        // Clear the display buffer
        oledDisplay.clearDisplay(); 

        // Draw each snowflake:
        for(int f=0; f< NUMFLAKES; f++) {
            oledDisplay.drawBitmap(icons[f][0], icons[f][1], logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, WHITE);
        }

        oledDisplay.display();
    }
}

void Display::updateSnowflakePositions() {
    for(int f=0; f< NUMFLAKES; f++) {
        icons[f][1] += icons[f][2];

        // If snowflake is off the bottom of the screen
        if (icons[f][1] >= oledDisplay.height()) {
            // Reinitialize to a random position, just off the top
            icons[f][0]   = random(1 - LOGO_WIDTH, oledDisplay.width());
            icons[f][1]   = -LOGO_HEIGHT;
            icons[f][2]   = random(100, 600)/50.0;
        }
    }
}

bool Display::isSecondPassed() {
    uint32_t currentMillis = millis();
    
    if (currentMillis - lastTimeUpdate >= 1000) {
        lastTimeUpdate = currentMillis;
        return true;
    }
    return false;
}

void Display::getElapsedTime(uint32_t& hours, uint32_t& minutes, uint32_t& seconds) {
    uint32_t totalMillis = millis();
    uint32_t totalSeconds = totalMillis / 1000;
    
    hours = totalSeconds / 3600;
    uint32_t remainingSeconds = totalSeconds % 3600;
    minutes = remainingSeconds / 60;
    seconds = remainingSeconds % 60;
}

void Display::renderTime(uint32_t hours, uint32_t minutes, uint32_t seconds) {
    // Clear display
    oledDisplay.clearDisplay();
    
    oledDisplay.setTextSize(1);
    oledDisplay.setTextColor(WHITE);

    // Set the cursor to the starting position
    oledDisplay.setCursor(0, 0);

    // Display the time
    oledDisplay.print("Uptime: ");
    if(hours < 10) oledDisplay.print('0');
    oledDisplay.print(hours);
    oledDisplay.print(':');
    if(minutes < 10) oledDisplay.print('0');
    oledDisplay.print(minutes);
    oledDisplay.print(':');
    if(seconds < 10) oledDisplay.print('0');
    oledDisplay.println(seconds);

    // Push the data to the OLED
    oledDisplay.display();
}

void Display::showTimeSinceStart() {
    if (isSecondPassed()) {
        uint32_t hours, minutes, seconds;
        getElapsedTime(hours, minutes, seconds);
        renderTime(hours, minutes, seconds);
    }
}
