#ifndef LIGHT_H
#define LIGHT_H

#include <Arduino.h>

class Light {
public:
    Light(int pin); 
    void init();
    void update();  // This method will be called every loop to handle pulsing

private:
    int pin;           
    int currentBrightness;
    int fadeAmount;
    uint32_t lastUpdate;   // Last time the brightness was updated
    const uint32_t interval = 30; // Time interval in ms for updating brightness
    void setBrightness(int brightness);
    void pulse();
};

#endif
