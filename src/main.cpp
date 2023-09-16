#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Display.h"

#define OLED_I2C_ADDR 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32


Adafruit_SSD1306 oledDisplay(OLED_WIDTH, OLED_HEIGHT, &Wire);

Display display(oledDisplay);
void setup() {
    pinMode(PC13, OUTPUT);
    Serial.begin(9600);

    if(!oledDisplay.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }

    oledDisplay.display();
    delay(2000);

    oledDisplay.clearDisplay();
    oledDisplay.display();

    display.init();
}

void loop() {
    display.animate();
}