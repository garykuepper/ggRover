/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 3 build: + I2C scan.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static void scanI2c() {
  Serial.println(F("I2C scan begin"));
  uint8_t found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  Serial.print(F("I2C scan end: "));
  Serial.print(found);
  Serial.println(F(" device(s)"));
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);

  ps4.begin();  // Wire.begin()

  // Give USB serial a moment to enumerate on the Pro Micro before the scan.
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}

  scanI2c();

  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 3");
  }
}

void loop() {
  // Stage 3: still passive.
}
