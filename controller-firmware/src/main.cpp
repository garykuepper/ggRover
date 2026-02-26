/**
 * @file main.cpp
 * @brief Main entry point for the Pro Micro Remote Controller firmware.
 */

#include <Arduino.h>
#include <Wire.h>
#include "Protocol.h"
#include "PS4Interface.h"
#include "Dashboard.h"

// Objects
PS4Interface ps4(0x29); // Example I2C address for USB Host
Dashboard dash(128, 64);

// Communication
ControlPacket txPacket;
TelemetryPacket rxPacket;

void setup() {
  Serial.begin(115200);   // USB Debug
  Serial1.begin(57600);  // XBee (Serial1 on Pro Micro)

  ps4.begin();
  if (!dash.begin()) {
    Serial.println("OLED Init Failed");
  } else {
    dash.showMessage("Rover Ready");
  }
}

void loop() {
  // 1. Poll Controller
  ps4.update();

  // 2. Prepare Command Packet
  txPacket.throttle = ps4.getThrottle();
  txPacket.steering = ps4.getSteering();
  txPacket.mode = 1; // Manual
  
  // 3. Send Commands
  static uint32_t lastTx = 0;
  if (millis() - lastTx > 50) { // 20Hz
    Serial1.write((uint8_t*)&txPacket, sizeof(ControlPacket));
    lastTx = millis();
  }

  // 4. Handle Telemetry
  if (Serial1.available() >= sizeof(TelemetryPacket)) {
    Serial1.readBytes((uint8_t*)&rxPacket, sizeof(TelemetryPacket));
    dash.update(rxPacket);
  }
}
