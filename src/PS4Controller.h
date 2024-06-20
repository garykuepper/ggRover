#ifndef PS4CONTROLLER_H
#define PS4CONTROLLER_H

#include <PS4Monitor.h>

class PS4Controller {
public:
    PS4Controller();
    void begin();
    void update();
    int getLX();
    int getLY();
    int getRX();
    int getRY();

private:
    PS4Monitor _ps4;
};

#endif
