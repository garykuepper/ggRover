#include "PS4Interface.h"

#include <Wire.h>

namespace {

// Centered (raw - 128) with a symmetric deadband. Returns 0 inside the band.
int16_t centeredWithDeadzone(uint8_t raw, uint8_t band) {
  int16_t v = static_cast<int16_t>(raw) - 128;
  if (v >= -static_cast<int16_t>(band) && v <= static_cast<int16_t>(band)) return 0;
  return v;
}

}  // namespace

PS4Interface::PS4Interface(uint8_t i2cAddr)
    : _pad(i2cAddr),
      _lastChangeMs(0),
      _okCount(0),
      _errCount(0),
      _lastStatus(0xFF),
      _prevAccelX(0),
      _prevAccelY(0) {}

void PS4Interface::begin() {
  Wire.begin();
  // Without this, a stuck SDA line (no pull-ups, missing slave, hung peripheral)
  // makes endTransmission() block forever and prevents the USB CDC 1200bps
  // auto-reset path from working — recovery then requires a manual double-tap.
  Wire.setWireTimeout(3000 /*us*/, true /*reset_with_timeout*/);
}

bool PS4Interface::poll() {
  _lastStatus = _pad.get_data();
  if (_lastStatus == PS4_OK) {
    _pad.decode_data();
    if (_pad.accel_x != _prevAccelX || _pad.accel_y != _prevAccelY) {
      _lastChangeMs = millis();
      _prevAccelX = _pad.accel_x;
      _prevAccelY = _pad.accel_y;
    }
    _okCount++;
    return true;
  }
  _errCount++;
  return false;
}

bool PS4Interface::connected() const {
  if (_lastStatus != PS4_OK) return false;
  return (millis() - _lastChangeMs) <= kStaleMs;
}

uint16_t PS4Interface::throttle() const {
  if (!connected()) return 500;
  // l_joystick_y: 0 = full up (forward). Invert so forward is positive.
  int16_t d = centeredWithDeadzone(_pad.l_joystick_y, kDeadzone);
  // Scale (-128..+127) -> (-500..+500), invert, then offset to (0..1000).
  int32_t scaled = static_cast<int32_t>(-d) * 500 / 128;
  int32_t out = 500 + scaled;
  if (out < 0) out = 0;
  if (out > 1000) out = 1000;
  return static_cast<uint16_t>(out);
}

int16_t PS4Interface::steering() const {
  if (!connected()) return 0;
  int16_t d = centeredWithDeadzone(_pad.r_joystick_x, kDeadzone);
  int32_t scaled = static_cast<int32_t>(d) * 500 / 128;
  if (scaled < -500) scaled = -500;
  if (scaled > 500) scaled = 500;
  return static_cast<int16_t>(scaled);
}
