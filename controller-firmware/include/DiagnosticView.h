/**
 * @file DiagnosticView.h
 * @brief Dense single-page SSD1306 diagnostic dashboard. Renders PS4 link
 *        status, raw inputs, mapped ControlPacket values, TX counters, and
 *        uptime. Independent of any TelemetryPacket — this view exists to
 *        prove the controller chain works without a rover.
 */

#ifndef DIAGNOSTIC_VIEW_H
#define DIAGNOSTIC_VIEW_H

#include <Adafruit_SSD1306.h>

#include "PS4Interface.h"
#include "Protocol.h"

class DiagnosticView {
 public:
  static constexpr uint8_t kWidth = 128;
  static constexpr uint8_t kHeight = 64;
  static constexpr uint8_t kI2cAddr = 0x3C;

  DiagnosticView();

  bool begin();

  // One-shot boot banner; safe to call before the main loop starts.
  void showBoot(const char* msg);

  // Render the dense diagnostic page. txRateHz extends the spec signature so
  // the 1-second rolling rate can be computed by the caller (not in render).
  void render(const PS4Interface& ps4,
              const ControlPacket& lastTx,
              uint32_t txCount,
              uint16_t txRateHz,
              uint32_t uptimeMs);

 private:
  Adafruit_SSD1306 _display;
};

#endif  // DIAGNOSTIC_VIEW_H
