#include <Arduino.h>
#include <DS4_I2C_CONTROL.h>
#define DEBUG false

unsigned long timer;
uint32_t baudRate = 115200;
uint8_t read_interval = 20;
DS4_I2C_CONTROL ds4 = DS4_I2C_CONTROL(0x29);

void showStatus()
{
  Serial.println((String)"PS4 left joystick value is x: " + ds4.l_joystick_x + " y: " + ds4.l_joystick_y);
  Serial.println((String)"PS4 right joystick value is x: " + ds4.r_joystick_x + " y: " + ds4.r_joystick_y);
  Serial.println((String)"PS4 R1 button is:  " + ds4.button_r1);
  Serial.println((String)"PS4 R2 button is:  " + ds4.button_r2);  
}

void setup()  {
  while( !Serial ); // sometimes necessary with Teensy 3 or Arduino Micro
  Serial.begin(baudRate);
  timer = 0;
  ds4.begin();
}

void loop()  {

  if (millis() > timer)
  {
    timer = millis() + read_interval;
    ds4.get_ps4();
    if (DEBUG) {
       Serial.println((String)"ps4_ok = " + ds4.ps4_ok);
    }   
    if (ds4.ps4_ok)
    {
      showStatus();
    }
  }
}

