#include "OLED.h"

OLED::OLED(int resetPin) : display(128, 32, &Wire, resetPin) {}

void OLED::begin() {
    if(!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);  // Infinite loop to halt execution as display couldn't start
    }
    display.display();
    delay(2000);
    display.clearDisplay();
}

void OLED::displayTime(unsigned long elapsedTime) {
    char timeBuffer[32];  
    snprintf(timeBuffer, sizeof(timeBuffer), "Time: %lu ms", elapsedTime);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.print(timeBuffer);
    display.display();
}
