#ifndef DISPLAY_H
#define DISPLAY_H

#include <Display.h>  // Include your specific display library

class Display {
public:
    Display();
    void init();
    void update();
    void clear();
    void showTimeSinceStart();
    void writeText(int x, int y, String text);

private:
    // Add your display object here
};

#endif
