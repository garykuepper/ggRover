/**
 * @file PS4Interface.h
 * @brief Thin adapter around Gamepad_PS4BT that exposes mapped, deadzoned,
 *        failsafed throttle/steering plus a const accessor to the raw pad
 *        state for the diagnostic view.
 */

#ifndef PS4_INTERFACE_H
#define PS4_INTERFACE_H

#include <Arduino.h>
#include <gamepad_ps4bt.h>

class PS4Interface {
 public:
  static constexpr uint8_t kDefaultAddr = 0x29;
  static constexpr uint8_t kDeadzone = 10;
  static constexpr uint32_t kStaleMs = 200;

  explicit PS4Interface(uint8_t i2cAddr = kDefaultAddr);

  void begin();

  // Reads + decodes one report. Updates connected/counters/last-ok timestamp.
  // Returns true iff get_data() returned PS4_OK.
  bool poll();

  // True iff the last poll was OK AND it happened within kStaleMs.
  bool connected() const;

  // True iff the failsafe is currently engaged (i.e. !connected()).
  bool failsafeActive() const { return !connected(); }

  // Mapped, deadzoned, failsafed values for the ControlPacket.
  uint16_t throttle() const;  // 0..1000, center 500
  int16_t  steering() const;  // -500..+500

  // Diagnostic accessors.
  const Gamepad_PS4BT& raw() const { return _pad; }
  uint8_t lastStatus() const { return _lastStatus; }
  uint32_t okCount() const { return _okCount; }
  uint32_t errCount() const { return _errCount; }

 private:
  // _lastChangeMs is the millis() of the most recent poll that produced a HID
  // report *different* from the previous one. The Hobbytronics adapter keeps
  // replaying the last cached report indefinitely after a BT disconnect, so
  // _lastStatus == PS4_OK alone is not a usable heartbeat — accel-byte change
  // is. Accel jitters every poll on a live controller (gravity + sensor noise),
  // so a stale value for >kStaleMs reliably means the link is dead.
  Gamepad_PS4BT _pad;
  uint32_t _lastChangeMs;
  uint32_t _okCount;
  uint32_t _errCount;
  uint8_t _lastStatus;
  uint8_t _prevAccelX;
  uint8_t _prevAccelY;
};

#endif  // PS4_INTERFACE_H
