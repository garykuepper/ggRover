#ifndef _PS4_MONITOR_H
#define _PS4_MONITOR_H

#include <DS4_I2C_CONTROL.h>
#include "Display.h"

class PS4Monitor {
public:
    PS4Monitor(uint8_t i2cAddr, Display& display);
    void begin();
    void update();

private:
    DS4_I2C_CONTROL ds4;
    Display& display;
    void displayPS4Status();
};

#endif
