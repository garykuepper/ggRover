// Blink.h
#ifndef BLINK_H
#define BLINK_H

#include <Arduino.h>

class Blink {
private:
    uint8_t pin;
    uint32_t interval;
    uint32_t lastToggleTime;
    bool ledState;

public:
    Blink(uint8_t _pin, uint32_t _interval);
    void toggle();
    void update();
    String getStatus() const;
    
};

#endif
