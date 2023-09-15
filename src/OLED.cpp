#include "OLED.h"

OLED::OLED(int resetPin) : display(128, 32, &Wire, resetPin) {}

void OLED::init() {
    if(!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    display.display();
}

void OLED::clearDisplay() {
    display.clearDisplay();
}

void OLED::print(const char* message) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(message);
    display.display();
}