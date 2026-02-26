/**
 * @file Odometry.h
 * @brief Class for tracking rover position using hardware timer-based encoders.
 */

#ifndef ODOMETRY_H
#define ODOMETRY_H

#include <Arduino.h>

/**
 * @class Odometry
 * @brief Handles encoder counting and distance calculations using STM32 timers.
 */
class Odometry {
public:
  /**
   * @brief Construct a new Odometry object.
   */
  Odometry();

  /**
   * @brief Initialize hardware timers for encoder feedback.
   */
  void begin();

  /**
   * @brief Reset encoder counts to zero.
   */
  void reset();

  /**
   * @brief Get the current left encoder count.
   * @return uint32_t Encoder ticks.
   */
  uint32_t getLeftTicks();

  /**
   * @brief Get the current right encoder count.
   * @return uint32_t Encoder ticks.
   */
  uint32_t getRightTicks();

  /**
   * @brief Calculate distance traveled in millimeters.
   * @return float Distance in mm.
   */
  float getDistance();

private:
  uint32_t _leftTicks;
  uint32_t _rightTicks;
};

#endif  // ODOMETRY_H
