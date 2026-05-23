# Controller-Side Bring-Up — Design

**Date:** 2026-05-23
**Scope:** `controller-firmware/` only. The rover does not exist yet.
**Status:** Approved (pending user spec review)

## 1. Goal

Prove the controller chain end-to-end on real hardware, with no rover required:

```
PS4 DualShock 4  --BT-->  USB BT dongle  --USB-->  Hobbytronics USB Host
                                                              |
                                                              | I2C @ 0x29
                                                              v
                                                       Arduino Pro Micro
                                                       /                \
                                                      /                  \
                                          (UART1 / 57600)            (I2C / 0x3C)
                                                /                          \
                                               v                            v
                                        XBee S2C (TX only)             SSD1306 OLED
```

Success = the operator can sit at a desk holding the controller, see live evidence on the OLED that (a) the PS4 is connected, (b) every link in the chain is alive, (c) ControlPackets are flowing out over the XBee at the configured cadence.

## 2. Non-Goals

- No rover-side firmware changes.
- No real telemetry RX path (rover doesn't exist). Skeleton hooks left in but inert.
- No CRC / packet framing yet — that's roadmap Phase 2.
- No reverse-throttle direction handling — protocol limitation, deferred to roadmap Phase 1's "Add direction control" task.
- No multi-page OLED, no PS4 button-driven mode switching. Single dense diagnostic page.

## 3. Hardware Inventory & Prerequisites

| Item | On hand | Notes |
|---|---|---|
| Pro Micro 5V / 16 MHz (SparkFun) | yes | `platformio.ini` currently misconfigured for 8 MHz variant — fix required |
| Hobbytronics USB Host adapter | yes | Already configured in I2C mode (`I2C 41`, `SERIAL OFF`, `HEX OFF`). Default I2C addr `0x29`. |
| USB Bluetooth dongle | yes | Already paired with the DS4; controller does NOT plug into USB Host directly |
| PS4 DualShock 4 | yes | Pair via PS+Share hold → fast-blink → solid color once paired |
| SSD1306 128×64 OLED | yes | I2C addr `0x3C` (already in code) |
| XBee S2C | yes | TX only for now; baud 57600 on `Serial1` |

### Prerequisite checks before flashing

- [ ] Confirm Pro Micro silkscreen reads `5V/16MHz`.
- [ ] Confirm USB BT dongle is in the Hobbytronics USB-A port and a fresh DS4 pair attempt produces a **solid** light bar.
- [ ] I2C pull-ups present on SDA/SCL. Pro Micro has weak internal pull-ups; if instability occurs, add external 4.7 kΩ to 5V (per Hobbytronics README).

## 4. Architecture

Three classes, single-threaded super-loop. No FreeRTOS, no scheduler.

```
+------------------+    +------------------+    +-------------------+
|   PS4Interface   |--->|     main.cpp     |--->|    XBee (UART)    |
| (Gamepad_PS4BT)  |    |   (super-loop)   |    +-------------------+
+------------------+    |                  |
                        |                  |    +-------------------+
                        |                  |--->|  DiagnosticView   |
                        |                  |    |    (SSD1306)      |
                        |                  |    +-------------------+
                        +------------------+
```

### Components

#### `PS4Interface` (thin adapter around `Gamepad_PS4BT`)

Wraps the library so:
- `main.cpp` sees a mapped `int throttle()` / `int steering()` for the `ControlPacket`.
- The diagnostic view sees the raw `Gamepad_PS4BT` state via a `const&` accessor.

```cpp
class PS4Interface {
 public:
  explicit PS4Interface(uint8_t i2cAddr = 0x29);
  void begin();
  bool poll();              // wraps get_data() + decode_data(); returns true on PS4_OK
  bool connected() const;   // last poll was OK
  uint16_t throttle() const;// 0..1000, deadzoned, failsafed
  int16_t  steering() const;// -500..+500, deadzoned, failsafed
  const Gamepad_PS4BT& raw() const;  // diagnostic accessor

 private:
  Gamepad_PS4BT _pad;
  bool _connected = false;
  uint8_t _lastStatus = 0xFF;
  uint32_t _okCount = 0;
  uint32_t _errCount = 0;
};
```

The current `PS4Interface` implementation (8-byte placeholder read, hardcoded `isButtonPressed`) is **deleted and replaced** by this design.

#### `DiagnosticView` (renames `Dashboard`)

The existing `Dashboard` class is repurposed. The class is renamed `DiagnosticView` to reflect intent. Its `update()` no longer takes a `TelemetryPacket` — it takes references to the things that actually exist in this phase:

```cpp
class DiagnosticView {
 public:
  DiagnosticView();
  bool begin();
  void render(const PS4Interface& ps4,
              const ControlPacket& lastTx,
              uint32_t txCount,
              uint32_t uptimeMs);
  void showBoot(const char* msg);
};
```

`update(const TelemetryPacket&)` is removed. The future rover-telemetry dashboard will be a *separate* class added later; the diagnostic view stays around forever as a permanent debug screen.

#### `main.cpp` super-loop

Three independent cadences driven by `millis()` deltas:

| Tick | Period | Action |
|---|---|---|
| PS4 poll | 20 ms (50 Hz) | `ps4.poll()` — matches library `POLL_INTERVAL` |
| XBee TX | 50 ms (20 Hz) | Build `ControlPacket` from `ps4.throttle()/steering()`, write to `Serial1`, bump `txCount` |
| OLED render | 200 ms (5 Hz) | `view.render(...)` — slow enough to avoid I2C blocking the PS4 poll |

## 5. Data Flow

```
DS4 sticks/buttons
  -> BT dongle -> USB Host firmware
    -> I2C read (Wire.requestFrom 0x29, 14 bytes)
      -> Gamepad_PS4BT::get_data() + decode_data()
        -> PS4Interface mapping (deadzone, scale, failsafe)
          -> ControlPacket {throttle, steering, mode=1, flags=0, checksum=0}
            -> Serial1.write(...) at 20 Hz
```

Side branch: `DiagnosticView::render(...)` reads `PS4Interface::raw()`, last `ControlPacket`, and counters once per 200 ms.

`Serial1` RX path: not implemented in this phase. The XBee will sit idle on RX.

## 6. OLED Layout (dense single page, Option A)

128×64 mono, default 6×8 font, 21 chars × 8 lines.

```
+---------------------+
|PS4:OK  BAT:87% S:42 |   line 0: PS4 link, controller battery, packet ok/err short
|LX:128 LY:042  TX:14 |   line 1: left stick raw + TX rate (Hz)
|RX:130 RY:128  PKT:N |   line 2: right stick raw + total tx packet count
|L2:000 R2:255  ERR:0 |   line 3: triggers + I2C error count
|Btns:X[ ] O[X] T[ ] |    line 4: face buttons
|     S[X] U[ ] D[X] |    line 5: shape + dpad
|THR:0500  STR:+0042  |   line 6: mapped values being sent
|Up:01:23     v0.1    |   line 7: uptime + firmware version
+---------------------+
```

- `PS4:OK` / `PS4:--` from `PS4Interface::connected()`.
- `BAT` from `Gamepad_PS4BT::battery` (0–255 → %).
- `TX` is rolling-window rate (last second). `PKT` is monotonic count.
- `ERR` is monotonic I2C / decode error count.
- Buttons rendered as `X[ ]` / `O[X]` etc. using the named members from the library.
- `THR/STR` are the *mapped* values actually being sent — gold-standard check that the input pipeline matches the output.

## 7. Input Mapping (tank-style)

```
Library raw      ->  Mapping                              ->  ControlPacket
l_joystick_y     ->  invert (up = forward), deadzone ±10  ->  throttle (0..1000)
                     center 128, linear scale
r_joystick_x     ->  deadzone ±10, center 128, linear     ->  steering (-500..+500)
```

Deadzone behavior: if `|raw - 128| <= 10`, output is centered (throttle = 500, steering = 0). Outside the deadzone, linear from center to the rail.

Reverse: not handled. Throttle below center maps to `< 500`; rover firmware currently treats throttle as unsigned PWM so reverse is invisible without protocol work (Phase 2). Documented limitation, not a bug.

Failsafe: if `PS4Interface::poll()` ever returns false OR no successful poll has occurred in the last 200 ms → next sent `ControlPacket` has `throttle = 500` (center / zero), `steering = 0`, `flags |= FAILSAFE_BIT` (new flag, value `0x01`).

## 8. Build & Dependency Changes

### `controller-firmware/platformio.ini`

```diff
 [env:sparkfun_promicro8]
+[env:sparkfun_promicro16]
 platform = atmelavr
-board = sparkfun_promicro8
+board = sparkfun_promicro16
 framework = arduino

 lib_deps =
     adafruit/Adafruit SSD1306@^2.5.9
     adafruit/Adafruit GFX Library@^1.11.9
-    # Note: Gamepad_PS4BT (Hobbytronics USB Host) may need manual installation
+    https://github.com/semuadmin/Gamepad_PS4BT.git

 build_flags =
     -I ../shared
```

(Environment name change reflects 5V/16MHz hardware; not a cosmetic rename.)

### Files touched

| File | Change |
|---|---|
| `controller-firmware/platformio.ini` | env name + board + lib_deps |
| `controller-firmware/include/PS4Interface.h` | rewrite per §4 |
| `controller-firmware/src/PS4Interface.cpp` | rewrite per §4 |
| `controller-firmware/include/Dashboard.h` | rename → `DiagnosticView.h`, signature change |
| `controller-firmware/src/Dashboard.cpp` | rename → `DiagnosticView.cpp`, render() rewrite |
| `controller-firmware/src/main.cpp` | super-loop with 3 cadences, failsafe, counter tracking |

## 9. Bring-Up Sequence (validation order)

The plan executes in stages so a failure at any stage is locally diagnosable.

1. **Build with new `platformio.ini`** — confirms board, library fetch, and that nothing else regressed.
2. **OLED hello world** — flash a `view.showBoot("ggRover ctrl v0.1")`, hold for 2 s, verify display works in isolation.
3. **I2C scan** — temporary `setup()` debug: log every responding I2C address over USB serial. Confirm `0x29` (USB Host) and `0x3C` (OLED) both present.
4. **PS4 poll without rendering** — verify USB serial prints raw stick values when controller is connected and moving.
5. **OLED diagnostic page** — full render path, with sticks moving live.
6. **XBee TX** — confirm 20 Hz cadence with a logic analyzer or second serial monitor on the XBee's UART side. Inspect bytes match `sizeof(ControlPacket)`.
7. **Failsafe** — disconnect controller mid-stream; confirm OLED flips to `PS4:--`, mapped THR/STR go to center/zero, FLAGS bit shows in TX bytes.

Each stage is independently committable; failures don't compound.

## 10. Open Questions

- Should the diagnostic page survive into the rover-online era, or be replaced by the telemetry view? **Recommendation:** keep it forever, add a future PS4-button-driven page toggle when telemetry view exists.

## 11. References

- Hobbytronics USB Host PS3/PS4 product: http://www.hobbytronics.co.uk/usb-host/ps3-ps4-controller-bluetooth
- `Gamepad_PS4BT` library: https://github.com/semuadmin/Gamepad_PS4BT
- Roadmap items advanced by this work: Phase 1 "Install USB Host library", "Parse I2C gamepad report", "Implement button support", "Validate OLED layout".
- Roadmap items deferred: Phase 1 "Add direction control" (protocol), Phase 2 CRC + framing, telemetry RX.
