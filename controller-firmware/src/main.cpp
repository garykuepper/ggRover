/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 4 build: + 50Hz PS4 poll
 *        and 5Hz USB-serial trace of raw stick state.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static constexpr uint32_t kPollPeriodMs = 20;    // 50 Hz
static constexpr uint32_t kTracePeriodMs = 200;  // 5 Hz

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

  ps4.begin();

  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}

  scanI2c();

  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 4");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastPoll = 0;
  static uint32_t lastTrace = 0;

  if (now - lastPoll >= kPollPeriodMs) {
    lastPoll = now;
    ps4.poll();
  }

  if (now - lastTrace >= kTracePeriodMs) {
    lastTrace = now;
    const Gamepad_PS4BT& p = ps4.raw();
    Serial.print(F("conn="));
    Serial.print(ps4.connected() ? '1' : '0');
    Serial.print(F(" st="));
    Serial.print(ps4.lastStatus(), HEX);
    Serial.print(F(" LX="));
    Serial.print(p.l_joystick_x);
    Serial.print(F(" LY="));
    Serial.print(p.l_joystick_y);
    Serial.print(F(" RX="));
    Serial.print(p.r_joystick_x);
    Serial.print(F(" RY="));
    Serial.print(p.r_joystick_y);
    Serial.print(F(" OK="));
    Serial.print(ps4.okCount());
    Serial.print(F(" ERR="));
    Serial.println(ps4.errCount());
  }
}
