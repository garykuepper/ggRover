/**
 * @file PS4Interface.h
 * @brief Class for interfacing with PS4 Bluetooth controller via Hobbytronics USB Host.
 */

#ifndef PS4_INTERFACE_H
#define PS4_INTERFACE_H

#include <Arduino.h>

/**
 * @class PS4Interface
 * @brief Handles gamepad inputs via I2C from the USB Host controller.
 */
class PS4Interface {
public:
  /**
   * @brief Construct a new PS4Interface object.
   * @param i2cAddr I2C address of the USB Host board.
   */
  PS4Interface(uint8_t i2cAddr);

  /**
   * @brief Initialize I2C communication.
   */
  void begin();

  /**
   * @brief Poll the controller for new data.
   */
  void update();

  /**
   * @brief Get the left joystick Y-axis (throttle).
   * @return int Throttle value (0 to 1000).
   */
  int getThrottle();

  /**
   * @brief Get the right joystick X-axis (steering).
   * @return int Steering value (-500 to 500).
   */
  int getSteering();

  /**
   * @brief Check if a specific button is pressed.
   * @param buttonId ID of the button.
   * @return true if pressed.
   */
  bool isButtonPressed(uint8_t buttonId);

private:
  uint8_t _addr;
  int _throttle;
  int _steering;
};

#endif  // PS4_INTERFACE_H
