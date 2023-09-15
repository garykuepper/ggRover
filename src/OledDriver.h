#ifndef OLEDDRIVER_H
#define OLEDDRIVER_H

#include "OLED.h"

class OLEDDriver {
public:
    void initialize();
    void run();  // Added run() method

private:
    void displayTimeSinceStart();  // Made this method private, since it's now called from run()
    OLED oledDisplay;
};

#endif
