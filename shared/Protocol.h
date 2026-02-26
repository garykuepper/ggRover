/**
 * @file Protocol.h
 * @brief Communication protocol definitions for XBee link between Rover and Controller.
 */

#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <stdint.h>

#pragma pack(push, 1)

/**
 * @struct ControlPacket
 * @brief Data sent from Controller to Rover.
 */
struct ControlPacket {
  uint16_t throttle;  ///< Forward/Backward speed (0-1000)
  int16_t steering;   ///< Left/Right steering (-500 to 500)
  uint8_t mode;       ///< Operational mode
  uint8_t flags;      ///< Utility flags (lights, horn, etc.)
  uint32_t checksum;  ///< CRC32 checksum for data integrity
};

/**
 * @struct TelemetryPacket
 * @brief Data sent from Rover back to Controller.
 */
struct TelemetryPacket {
  float batteryVoltage;  ///< Current battery voltage
  float roll;            ///< IMU Roll angle
  float pitch;           ///< IMU Pitch angle
  float yaw;             ///< IMU Heading/Yaw
  uint32_t encoderL;     ///< Left wheel encoder counts
  uint32_t encoderR;     ///< Right wheel encoder counts
  float temperature;     ///< ECS BME280 temperature
  float pressure;        ///< ECS BME280 pressure
  uint32_t distance;    ///< ToF sensor distance in mm
  uint32_t checksum;     ///< CRC32 checksum for data integrity
};

#pragma pack(pop)

#endif  // SHARED_PROTOCOL_H
