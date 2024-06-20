#ifndef PS4CONTROLLER_H
#define PS4CONTROLLER_H

#include "DS4_I2C_CONTROL.h"

class PS4Controller {
public:
    PS4Controller(uint8_t address = 0x29);
    void begin();
    void update();
    uint8_t getLX();
    uint8_t getLY();
    uint8_t getRX();
    uint8_t getRY();

private:
    DS4_I2C_CONTROL _ps4;
};

#endif
