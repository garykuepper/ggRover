/**
 * @file Dashboard.h
 * @brief Class for managing the SSD1306 OLED dashboard.
 */

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Adafruit_SSD1306.h>
#include "Protocol.h"

/**
 * @class Dashboard
 * @brief Displays rover telemetry and controller status on the OLED.
 */
class Dashboard {
public:
  /**
   * @brief Construct a new Dashboard object.
   * @param width Screen width in pixels.
   * @param height Screen height in pixels.
   */
  Dashboard(uint8_t width, uint8_t height);

  /**
   * @brief Initialize the OLED display.
   * @return true if successful.
   */
  bool begin();

  /**
   * @brief Update the display with new telemetry.
   * @param data The telemetry packet received from the rover.
   */
  void update(const TelemetryPacket& data);

  /**
   * @brief Display a splash screen or error message.
   * @param msg Message to display.
   */
  void showMessage(const char* msg);

private:
  Adafruit_SSD1306 _display;
};

#endif  // DASHBOARD_H
