// BlinkDriver.h
#ifndef BLINK_DRIVER_H
#define BLINK_DRIVER_H

#include "Blink.h"

class BlinkDriver {
private:
    Blink* blinker;
    String lastStatus;
public:
    BlinkDriver(uint8_t pin);
    void run();
    void printStatusChange();
};

#endif
