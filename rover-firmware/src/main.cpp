/**
 * @file main.cpp
 * @brief Main entry point for the STM32 Rover firmware.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_VL53L0X.h>
#include "Protocol.h"
#include "Drivetrain.h"
#include "Odometry.h"
#include "HeadingSystem.h"

// Hardware Pin Definitions
#define MOTOR_FL_PWM PA0
#define MOTOR_FR_PWM PA1
#define MOTOR_RL_PWM PA2
#define MOTOR_RR_PWM PA3

// Objects
Drivetrain drive(MOTOR_FL_PWM, MOTOR_FR_PWM, MOTOR_RL_PWM, MOTOR_RR_PWM);
Odometry odo;
HeadingSystem heading;
Adafruit_BME280 bme;
Adafruit_VL53L0X tof = Adafruit_VL53L0X();

// Communication
ControlPacket rxPacket;
TelemetryPacket txPacket;

void setup() {
  // Serial.begin(115200);   // USB Debug (Disabled to save FLASH)
  Serial1.begin(57600);  // XBee

  drive.begin();
  odo.begin();

  if (!heading.begin()) {
    Serial1.println("IMU Init Failed");
  }

  if (!bme.begin(0x76)) {
    Serial1.println("BME280 Init Failed");
  }

  if (!tof.begin()) {
    Serial1.println("VL53L0X Init Failed");
  } else {
    tof.startRangeContinuous();
  }

}

void loop() {
  // 1. Update Sensors
  heading.update();
  
  // 2. Handle Communication (Example skeleton)
  if (Serial1.available() >= sizeof(ControlPacket)) {
    Serial1.readBytes((uint8_t*)&rxPacket, sizeof(ControlPacket));
    drive.drive(rxPacket.throttle, rxPacket.steering);
  }

  // 3. Prepare Telemetry
  txPacket.batteryVoltage = 12.6f; // Placeholder
  txPacket.yaw = heading.getYaw();
  txPacket.pitch = heading.getPitch();
  txPacket.roll = heading.getRoll();
  txPacket.encoderL = odo.getLeftTicks();
  txPacket.encoderR = odo.getRightTicks();
  txPacket.distance = tof.readRange();
  txPacket.temperature = bme.readTemperature();
  txPacket.pressure = bme.readPressure();

  // 4. Send Telemetry
  static uint32_t lastTx = 0;
  if (millis() - lastTx > 100) { // 10Hz
    Serial1.write((uint8_t*)&txPacket, sizeof(TelemetryPacket));
    lastTx = millis();
  }
}
