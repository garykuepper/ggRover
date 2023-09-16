// PS4Monitor.h
#ifndef PS4MONITOR_H
#define PS4MONITOR_H

#include "DS4_I2C_CONTROL.h"

class PS4Monitor {
public:
    PS4Monitor(uint8_t address = 0x29); // Default address set to 0x29
    void begin();
    void update();
    int getLX();
    int getLY();
    void showStatus();

private:
    DS4_I2C_CONTROL ds4;       

};

#endif // PS4Monitor_h