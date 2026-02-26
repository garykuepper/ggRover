/**
 * @file Drivetrain.h
 * @brief Class for controlling the 4-wheel DC motor drivetrain.
 */

#ifndef DRIVETRAIN_H
#define DRIVETRAIN_H

#include <Arduino.h>

/**
 * @class Drivetrain
 * @brief Manages motor speeds and directions for a 4-wheel rover.
 */
class Drivetrain {
public:
  /**
   * @brief Construct a new Drivetrain object.
   * @param flPin Front Left motor PWM pin.
   * @param frPin Front Right motor PWM pin.
   * @param rlPin Rear Left motor PWM pin.
   * @param rrPin Rear Right motor PWM pin.
   */
  Drivetrain(int flPin, int frPin, int rlPin, int rrPin);

  /**
   * @brief Initialize the motor controller hardware.
   */
  void begin();

  /**
   * @brief Drive the rover with given throttle and steering.
   * @param throttle Speed from -100 to 100.
   * @param steering Turn factor from -100 to 100.
   */
  void drive(int throttle, int steering);

  /**
   * @brief Stop all motors immediately.
   */
  void stop();

private:
  int _flPin, _frPin, _rlPin, _rrPin;
};

#endif  // DRIVETRAIN_H
