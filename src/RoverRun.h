#ifndef ROVERRUN_H
#define ROVERRUN_H

class RoverRun 
{
private:
    unsigned long previousMillis;
    const long interval = 10;

public:
    RoverRun();
    void run();
};

#endif