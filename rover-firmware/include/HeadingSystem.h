/**
 * @file HeadingSystem.h
 * @brief Class for managing MPU-9250 IMU using DMP for stable heading.
 */

#ifndef HEADING_SYSTEM_H
#define HEADING_SYSTEM_H

#include <SparkFunMPU9250-DMP.h>

/**
 * @class HeadingSystem
 * @brief Interfaces with MPU-9250 to provide Euler angles (Yaw, Pitch, Roll).
 */
class HeadingSystem {
public:
  /**
   * @brief Construct a new Heading System object.
   */
  HeadingSystem();

  /**
   * @brief Initialize IMU and DMP.
   * @return true if successful, false otherwise.
   */
  bool begin();

  /**
   * @brief Update sensor data from the FIFO buffer.
   */
  void update();

  /**
   * @brief Get current Yaw.
   * @return float Yaw in degrees.
   */
  float getYaw();

  /**
   * @brief Get current Pitch.
   * @return float Pitch in degrees.
   */
  float getPitch();

  /**
   * @brief Get current Roll.
   * @return float Roll in degrees.
   */
  float getRoll();

private:
  MPU9250_DMP _imu;
};

#endif  // HEADING_SYSTEM_H
