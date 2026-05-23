/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 2 build: OLED boot banner only.
 *        Subsequent commits add I2C scan, PS4 poll, render, XBee TX.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);

  ps4.begin();
  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 2");
  }
}

void loop() {
  // Stage 2: nothing yet. The boot banner is the entire observable behavior.
}
