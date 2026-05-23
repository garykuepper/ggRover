# Controller-Side Bring-Up Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Bring up the PS4 → Pro Micro → XBee + diagnostic OLED chain end-to-end on real hardware, without any rover present.

**Architecture:** Single-threaded super-loop on a SparkFun Pro Micro 5V/16MHz. Three independent `millis()`-driven cadences: 50 Hz PS4 poll over I2C (Hobbytronics USB Host @ 0x29 via the `Gamepad_PS4BT` library), 20 Hz `ControlPacket` TX over `Serial1` to the XBee, and 5 Hz dense diagnostic render to the SSD1306 OLED @ 0x3C. `PS4Interface` is a thin adapter that owns deadzone + mapping + failsafe state; `DiagnosticView` replaces the old telemetry-shaped `Dashboard`.

**Tech Stack:** PlatformIO (`atmelavr` platform, Arduino framework), `Adafruit_SSD1306` + `Adafruit_GFX`, `Gamepad_PS4BT` (https://github.com/semuadmin/Gamepad_PS4BT, header-at-repo-root, lowercase filenames), shared header-only `Protocol.h`.

**Spec:** `docs/superpowers/specs/2026-05-23-controller-bringup-design.md`

## On testing in this plan

This firmware has no host-side test suite and the project policy is on-device validation only. The "test" for each task is one of:

1. **`pio run`** — compile gate. Used after any code change.
2. **`pio check`** — clang-tidy gate at high severity. Used at the end.
3. **A specific hardware observation** — written explicitly per task (what to watch on OLED, USB serial, logic analyzer, etc.).

This plan follows the bring-up sequence in spec §9: each task adds one subsystem and ends with an observation that confirms it works in isolation before the next is layered on. Do not skip the observation step — silent compile success is not "done" for firmware.

## File map

| File | Disposition |
|---|---|
| `controller-firmware/platformio.ini` | Modify: env name + board + lib_deps |
| `controller-firmware/include/PS4Interface.h` | Rewrite |
| `controller-firmware/src/PS4Interface.cpp` | Rewrite |
| `controller-firmware/include/Dashboard.h` | **Delete** (replaced by `DiagnosticView.h`) |
| `controller-firmware/src/Dashboard.cpp` | **Delete** (replaced by `DiagnosticView.cpp`) |
| `controller-firmware/include/DiagnosticView.h` | Create |
| `controller-firmware/src/DiagnosticView.cpp` | Create |
| `controller-firmware/src/main.cpp` | Rewrite over several tasks (one cadence at a time) |
| `shared/Protocol.h` | Modify: add `FAILSAFE_BIT` |

**Note on a deviation from the spec:** spec §6 says the OLED `TX` field is a "rolling-window rate (last second)". The spec's `DiagnosticView::render` signature only takes `txCount` and `uptimeMs`. To support the rate without recomputing it inside `render` (which fires at 5 Hz, not 1 Hz), this plan extends the signature to also accept a precomputed `uint16_t txRateHz`. This is the minimum change that matches §6.

---

## Task 0: Prerequisites and clean baseline

**Files:** none (environment-only)

- [ ] **Step 1: Confirm `pio` is on PATH in this shell**

Run: `pio --version`
Expected: prints a version line, e.g. `PlatformIO Core, version 6.x.x`.

If "command not found": add `~/.platformio/penv/Scripts` (Windows) to PATH for this session and try again. If Cursor's integrated terminal still can't see `pio`, restart Cursor.

- [ ] **Step 2: Confirm clean working tree on `main`**

Run: `git status`
Expected: `On branch main` and `nothing to commit` (the `.cache/clangd/` modifications that existed at the start of the prior session should already have been ignored by commit `d9fe60d`).

If dirty with unrelated changes: stash before starting. Do not proceed otherwise.

- [ ] **Step 3: Bench check — confirm hardware matches the spec**

Verify on the bench (not in code):
- Pro Micro silkscreen reads `5V/16MHz`.
- Hobbytronics USB Host adapter is in I2C mode (`I2C 41`, `SERIAL OFF`, `HEX OFF` per spec §3).
- USB Bluetooth dongle is plugged into the USB Host's USB-A port (the DS4 does NOT plug into the host directly).
- SSD1306 + USB Host both wired to the Pro Micro's I2C (D2=SDA, D3=SCL on a 5V Pro Micro).
- XBee RX line connected to Pro Micro `TX1` (pin 1).

If any item fails, stop and resolve before flashing.

---

## Task 1: PlatformIO config — board + library

**Files:**
- Modify: `controller-firmware/platformio.ini`

- [ ] **Step 1: Replace `platformio.ini` contents**

Full replacement of `controller-firmware/platformio.ini`:

```ini
[env:sparkfun_promicro16]
platform = atmelavr
board = sparkfun_promicro16
framework = arduino

lib_deps =
    adafruit/Adafruit SSD1306@^2.5.9
    adafruit/Adafruit GFX Library@^1.11.9
    https://github.com/semuadmin/Gamepad_PS4BT.git

build_flags =
    -I ../shared

check_tool = clangtidy
check_flags =
    clangtidy: --checks=-*,bugprone-*,modernize-*,readability-*,performance-*
```

The env name change (`promicro8` → `promicro16`) is load-bearing — it picks the right F_CPU and avr-gcc flags for the 16 MHz / 5 V variant.

- [ ] **Step 2: Verify the build still passes with the unchanged old sources**

Run: `pio run -d controller-firmware`
Expected: a successful build. PlatformIO will fetch `Gamepad_PS4BT` from git on first run; expect a "Library Manager: Installing" line. The build should succeed because nothing yet references the new library — only `Adafruit_SSD1306` (used by the old `Dashboard`) is touched.

If the build fails complaining about `Gamepad_PS4BT.h` not found: that's not the failure mode here (no source references it yet). Any failure indicates an unrelated regression — stop and inspect.

- [ ] **Step 3: Commit**

```bash
git add controller-firmware/platformio.ini
git commit -m "controller: switch to sparkfun_promicro16 env and add Gamepad_PS4BT lib_dep"
```

This satisfies spec §9 stage 1 ("Build with new platformio.ini").

---

## Task 2: PS4Interface rewrite

**Files:**
- Modify: `controller-firmware/include/PS4Interface.h` (full rewrite)
- Modify: `controller-firmware/src/PS4Interface.cpp` (full rewrite)

This task introduces the wrapper but does not yet wire it into `main.cpp`. The compile gate alone proves the new header compiles against the actual `Gamepad_PS4BT` API.

- [ ] **Step 1: Replace `controller-firmware/include/PS4Interface.h`**

```cpp
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
  Gamepad_PS4BT _pad;
  uint32_t _lastOkMs;
  uint32_t _okCount;
  uint32_t _errCount;
  uint8_t _lastStatus;
};

#endif  // PS4_INTERFACE_H
```

The header references `gamepad_ps4bt.h` (lowercase) because that is the actual filename in the upstream repo.

- [ ] **Step 2: Replace `controller-firmware/src/PS4Interface.cpp`**

```cpp
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
      _lastOkMs(0),
      _okCount(0),
      _errCount(0),
      _lastStatus(0xFF) {}

void PS4Interface::begin() {
  Wire.begin();
}

bool PS4Interface::poll() {
  _lastStatus = _pad.get_data();
  if (_lastStatus == PS4_OK) {
    _pad.decode_data();
    _lastOkMs = millis();
    _okCount++;
    return true;
  }
  _errCount++;
  return false;
}

bool PS4Interface::connected() const {
  if (_lastStatus != PS4_OK) return false;
  return (millis() - _lastOkMs) <= kStaleMs;
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
```

- [ ] **Step 3: Verify the build**

Run: `pio run -d controller-firmware`
Expected: build fails — `main.cpp` still calls `ps4.update()`, `ps4.getThrottle()`, `ps4.getSteering()`, `ps4.isButtonPressed()`, none of which exist in the new header.

This failure is **expected and informative** — it tells us exactly what main.cpp still needs to be rewritten to consume. Do **not** add shim methods to keep the old main.cpp compiling; main.cpp is rewritten in Task 4 and beyond.

- [ ] **Step 4: Stage the new files but do not commit yet**

Hold the commit until Task 3 also lands, so the intermediate state never appears in history with a broken `main.cpp`. Just `git add` and continue:

```bash
git add controller-firmware/include/PS4Interface.h controller-firmware/src/PS4Interface.cpp
```

---

## Task 3: Replace Dashboard with DiagnosticView

**Files:**
- Delete: `controller-firmware/include/Dashboard.h`
- Delete: `controller-firmware/src/Dashboard.cpp`
- Create: `controller-firmware/include/DiagnosticView.h`
- Create: `controller-firmware/src/DiagnosticView.cpp`

The class is intentionally renamed (not just modified) because its responsibility has changed — it no longer renders a `TelemetryPacket`.

- [ ] **Step 1: Delete the old Dashboard files**

```bash
git rm controller-firmware/include/Dashboard.h controller-firmware/src/Dashboard.cpp
```

- [ ] **Step 2: Create `controller-firmware/include/DiagnosticView.h`**

```cpp
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
```

- [ ] **Step 3: Create `controller-firmware/src/DiagnosticView.cpp`**

```cpp
#include "DiagnosticView.h"

#include <Wire.h>

namespace {

// Formats "MM:SS" (capped at 99:59) for the uptime field. dst must hold >=6.
void formatMmSs(char* dst, uint32_t ms) {
  uint32_t s = ms / 1000UL;
  uint16_t mm = static_cast<uint16_t>(s / 60UL);
  uint16_t ss = static_cast<uint16_t>(s % 60UL);
  if (mm > 99) mm = 99;
  snprintf(dst, 6, "%02u:%02u", mm, ss);
}

// Renders a face/dpad button as "L[X]" or "L[ ]".
void appendButton(char* dst, char glyph, uint8_t pressed) {
  dst[0] = glyph;
  dst[1] = '[';
  dst[2] = pressed ? 'X' : ' ';
  dst[3] = ']';
  dst[4] = ' ';
}

}  // namespace

DiagnosticView::DiagnosticView()
    : _display(kWidth, kHeight, &Wire, -1) {}

bool DiagnosticView::begin() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, kI2cAddr)) return false;
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.display();
  return true;
}

void DiagnosticView::showBoot(const char* msg) {
  _display.clearDisplay();
  _display.setCursor(0, 0);
  _display.println(F("ggRover ctrl"));
  _display.println(msg);
  _display.display();
}

void DiagnosticView::render(const PS4Interface& ps4,
                            const ControlPacket& lastTx,
                            uint32_t txCount,
                            uint16_t txRateHz,
                            uint32_t uptimeMs) {
  const Gamepad_PS4BT& p = ps4.raw();
  char line[22];

  _display.clearDisplay();
  _display.setCursor(0, 0);

  // Line 0: PS4:OK  BAT:nn%  S:status-hex
  uint16_t batPct = (static_cast<uint16_t>(p.battery) * 100u) / 255u;
  snprintf(line, sizeof(line), "PS4:%s BAT:%02u%% S:%02X",
           ps4.connected() ? "OK" : "--",
           batPct,
           ps4.lastStatus());
  _display.println(line);

  // Line 1: LX:nnn LY:nnn  TX:rate
  snprintf(line, sizeof(line), "LX:%03u LY:%03u  TX:%02u",
           p.l_joystick_x, p.l_joystick_y, txRateHz);
  _display.println(line);

  // Line 2: RX:nnn RY:nnn  PKT:count (5 digits, rolls past 99999)
  snprintf(line, sizeof(line), "RX:%03u RY:%03u  PKT:%lu",
           p.r_joystick_x, p.r_joystick_y,
           static_cast<unsigned long>(txCount % 100000UL));
  _display.println(line);

  // Line 3: L2:nnn R2:nnn  ERR:count
  snprintf(line, sizeof(line), "L2:%03u R2:%03u  ERR:%lu",
           p.l2, p.r2,
           static_cast<unsigned long>(ps4.errCount() % 100000UL));
  _display.println(line);

  // Line 4: face buttons  X[ ] O[ ] T[ ]
  char btnLine[22] = {0};
  appendButton(&btnLine[0],  'X', p.button_x);
  appendButton(&btnLine[5],  'O', p.button_circle);
  appendButton(&btnLine[10], 'T', p.button_triangle);
  btnLine[14] = '\0';
  _display.println(btnLine);

  // Line 5: square + dpad up/down
  char btnLine2[22] = {0};
  appendButton(&btnLine2[0],  'S', p.button_square);
  appendButton(&btnLine2[5],  'U', p.button_up);
  appendButton(&btnLine2[10], 'D', p.button_down);
  btnLine2[14] = '\0';
  _display.println(btnLine2);

  // Line 6: mapped THR/STR being sent
  snprintf(line, sizeof(line), "THR:%04u  STR:%+05d",
           lastTx.throttle, lastTx.steering);
  _display.println(line);

  // Line 7: uptime + version
  char mmss[6];
  formatMmSs(mmss, uptimeMs);
  snprintf(line, sizeof(line), "Up:%s     v0.1", mmss);
  _display.println(line);

  _display.display();
}
```

- [ ] **Step 4: Verify the build still fails on `main.cpp` (only)**

Run: `pio run -d controller-firmware`
Expected: failure that mentions `Dashboard.h: No such file` or `class Dashboard` — coming from `main.cpp`. The new files themselves must compile cleanly.

If the failure mentions `DiagnosticView.cpp` or `PS4Interface.cpp`, fix the source there before proceeding — `main.cpp` is still expected to fail.

- [ ] **Step 5: Stage and commit Tasks 2 + 3 together with Task 4**

Hold the commit. The minimal commit-able unit is "all three rewrites + a main.cpp that compiles" — which is what Task 4 will produce.

```bash
git add controller-firmware/include/DiagnosticView.h controller-firmware/src/DiagnosticView.cpp
```

---

## Task 4: Minimal main.cpp — OLED boot banner only

**Files:**
- Modify: `controller-firmware/src/main.cpp` (full rewrite, minimal)

This is the smallest main.cpp that compiles against the new headers. Spec §9 stage 2: OLED hello world.

- [ ] **Step 1: Replace `controller-firmware/src/main.cpp`**

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 2 build: OLED boot banner only.
 *        Subsequent commits add I2C scan, PS4 poll, render, XBee TX.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);

  ps4.begin();
  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 2");
  }
}

void loop() {
  // Stage 2: nothing yet. The boot banner is the entire observable behavior.
}
```

- [ ] **Step 2: Build**

Run: `pio run -d controller-firmware`
Expected: clean build. The `Gamepad_PS4BT` library is now genuinely linked (via `PS4Interface`); first build after this change may rebuild more than usual.

- [ ] **Step 3: Commit the four-file rewrite as one unit**

```bash
git add controller-firmware/src/main.cpp
git commit -m "controller: rewrite PS4Interface, replace Dashboard with DiagnosticView, minimal main"
```

- [ ] **Step 4: Flash and observe (spec §9 stage 2)**

Run: `pio run -d controller-firmware --target upload`
Expected on hardware: the OLED shows two lines — `ggRover ctrl` and `boot stage 2` — within ~2 s of reset, then stays.

Pass criteria:
- OLED text is sharp (no smearing, no flicker).
- No reset loop (Pro Micro's RX/TX LEDs do not blink continuously).

If OLED is blank: check 0x3C wiring, power, pull-ups. If text is garbled: probable I2C contention with the USB Host adapter — confirm Task 0 step 3 again.

Do **not** continue to Task 5 until this passes.

---

## Task 5: Add the I2C scan (stage 3)

**Files:**
- Modify: `controller-firmware/src/main.cpp`

Adds a one-shot scan that prints to USB serial, so we can confirm both 0x29 (USB Host) and 0x3C (OLED) before relying on either.

- [ ] **Step 1: Add `scanI2c()` to `main.cpp` and call it from `setup()`**

Full replacement of `controller-firmware/src/main.cpp`:

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 3 build: + I2C scan.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static void scanI2c() {
  Serial.println(F("I2C scan begin"));
  uint8_t found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  Serial.print(F("I2C scan end: "));
  Serial.print(found);
  Serial.println(F(" device(s)"));
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);

  ps4.begin();  // Wire.begin()

  // Give USB serial a moment to enumerate on the Pro Micro before the scan.
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}

  scanI2c();

  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 3");
  }
}

void loop() {
  // Stage 3: still passive.
}
```

`scanI2c()` is marked temporary in spirit — it stays through stages 3–6 because it's a cheap sanity check on every reset, and is removed in Task 10.

- [ ] **Step 2: Build, flash, and observe (spec §9 stage 3)**

Run: `pio run -d controller-firmware --target upload`

Then open a USB serial monitor at 115200:

Run: `pio device monitor -d controller-firmware -b 115200`

Expected serial output within ~3 s of reset:
```
I2C scan begin
  found 0x29
  found 0x3C
I2C scan end: 2 device(s)
```

Pass criteria: both `0x29` and `0x3C` appear. Order does not matter.

If only `0x3C` appears: USB Host adapter is not on the bus — check power, address-jumper config (default is 0x29), and that it's in I2C mode (`I2C 41`).

If only `0x29` appears: OLED missing — check its power and pull-ups.

If neither appears: SDA/SCL swap, broken pull-ups, or `Wire.begin()` not actually called. Inspect `PS4Interface::begin()`.

- [ ] **Step 3: Commit**

```bash
git add controller-firmware/src/main.cpp
git commit -m "controller: add I2C scan to setup for stage-3 bring-up"
```

---

## Task 6: Add PS4 polling with USB-serial trace (stage 4)

**Files:**
- Modify: `controller-firmware/src/main.cpp`

Adds the 50 Hz PS4 poll cadence and prints raw stick values once per 200 ms to USB serial. No OLED rendering of pad state yet; no XBee TX yet.

- [ ] **Step 1: Add poll + serial-trace cadences to `loop()`**

Full replacement of `controller-firmware/src/main.cpp`:

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 4 build: + 50Hz PS4 poll
 *        and 5Hz USB-serial trace of raw stick state.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static constexpr uint32_t kPollPeriodMs = 20;   // 50 Hz
static constexpr uint32_t kTracePeriodMs = 200; // 5 Hz

static void scanI2c() {
  Serial.println(F("I2C scan begin"));
  uint8_t found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  Serial.print(F("I2C scan end: "));
  Serial.print(found);
  Serial.println(F(" device(s)"));
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);
  ps4.begin();
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}
  scanI2c();
  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 4");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastPoll = 0;
  static uint32_t lastTrace = 0;

  if (now - lastPoll >= kPollPeriodMs) {
    lastPoll = now;
    ps4.poll();
  }

  if (now - lastTrace >= kTracePeriodMs) {
    lastTrace = now;
    const Gamepad_PS4BT& p = ps4.raw();
    Serial.print(F("conn="));
    Serial.print(ps4.connected() ? '1' : '0');
    Serial.print(F(" st="));
    Serial.print(ps4.lastStatus(), HEX);
    Serial.print(F(" LX="));
    Serial.print(p.l_joystick_x);
    Serial.print(F(" LY="));
    Serial.print(p.l_joystick_y);
    Serial.print(F(" RX="));
    Serial.print(p.r_joystick_x);
    Serial.print(F(" RY="));
    Serial.print(p.r_joystick_y);
    Serial.print(F(" OK="));
    Serial.print(ps4.okCount());
    Serial.print(F(" ERR="));
    Serial.println(ps4.errCount());
  }
}
```

- [ ] **Step 2: Build, flash, observe (spec §9 stage 4)**

```
pio run -d controller-firmware --target upload
pio device monitor -d controller-firmware -b 115200
```

Pair the DS4 first (PS + Share until fast blink; should go solid once Hobbytronics adopts it).

Expected serial output once paired:
- Trace lines at ~5 Hz.
- `conn=1`, `st=0` when controller is connected.
- `LX/LY/RX/RY` near 128 when sticks are released.
- Move left stick fully up → `LY` drops toward 0; fully down → `LY` rises toward 255.
- `OK` counter increments roughly 250/sec (50 Hz). `ERR` stays at 0 or grows very slowly.

Pass criteria:
- Stick motion is reflected in the trace within one trace period.
- `ERR` is not growing faster than `OK` (occasional I2C glitch acceptable).

If `conn=0` persistently: DS4 isn't paired or the USB Host isn't seeing it. Re-pair (hold PS+Share 5 s, wait for solid color). Verify with the USB Host's status LED if it has one.

If `OK=0` and `ERR` climbing fast: I2C transaction is failing — check pull-ups; try a slower bus by adding `Wire.setClock(100000);` after `Wire.begin()` in `PS4Interface::begin()` and re-run.

- [ ] **Step 3: Commit**

```bash
git add controller-firmware/src/main.cpp
git commit -m "controller: add 50Hz PS4 poll and 5Hz USB-serial trace"
```

---

## Task 7: Add OLED render + ControlPacket assembly + FAILSAFE_BIT (stage 5)

**Files:**
- Modify: `shared/Protocol.h` (add `FAILSAFE_BIT`)
- Modify: `controller-firmware/src/main.cpp`

This task builds the `ControlPacket` from `PS4Interface` outputs and renders the full diagnostic page. **No XBee TX yet** — `Serial1` stays silent so failures show on-screen before they hit the wire.

- [ ] **Step 1: Add `FAILSAFE_BIT` to `shared/Protocol.h`**

After the closing `#pragma pack(pop)` line and before `#endif`, insert:

```cpp
/// ControlPacket.flags bit values.
#define FAILSAFE_BIT 0x01
```

The spec missed listing `Protocol.h` in §8 but §7 names `FAILSAFE_BIT` with value `0x01` for the controller→rover flag. Adding it to the shared header is the right home — the rover will need the same name later when it decodes the flag.

- [ ] **Step 2: Add ControlPacket build + 5 Hz render to `main.cpp`**

Full replacement of `controller-firmware/src/main.cpp`:

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 5 build: + ControlPacket
 *        assembly and 5Hz OLED diagnostic render. No XBee TX yet.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static constexpr uint32_t kPollPeriodMs = 20;    // 50 Hz
static constexpr uint32_t kRenderPeriodMs = 200; // 5 Hz

static ControlPacket g_lastTx = {500, 0, 1, 0, 0};
static uint32_t g_txCount = 0;
static uint16_t g_txRateHz = 0;

static void scanI2c() {
  Serial.println(F("I2C scan begin"));
  uint8_t found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  Serial.print(F("I2C scan end: "));
  Serial.print(found);
  Serial.println(F(" device(s)"));
}

static void buildControlPacket() {
  g_lastTx.throttle = ps4.throttle();
  g_lastTx.steering = ps4.steering();
  g_lastTx.mode = 1;  // manual
  g_lastTx.flags = ps4.failsafeActive() ? FAILSAFE_BIT : 0;
  g_lastTx.checksum = 0;  // CRC deferred to roadmap phase 2
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);
  ps4.begin();
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}
  scanI2c();
  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 5");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastPoll = 0;
  static uint32_t lastRender = 0;

  if (now - lastPoll >= kPollPeriodMs) {
    lastPoll = now;
    ps4.poll();
    buildControlPacket();  // keep g_lastTx fresh for the renderer
  }

  if (now - lastRender >= kRenderPeriodMs) {
    lastRender = now;
    view.render(ps4, g_lastTx, g_txCount, g_txRateHz, now);
  }
}
```

- [ ] **Step 3: Build, flash, observe (spec §9 stage 5)**

```
pio run -d controller-firmware --target upload
```

Pass criteria on the OLED:
- All 8 lines of the layout in spec §6 are present and legible.
- `PS4:OK` when controller connected; `BAT:nn%` shows a plausible battery percent.
- Moving left stick changes `LY:` (raw) and `THR:` (mapped) simultaneously; centered stick shows `LY:128` and `THR:0500`.
- Moving right stick changes `RX:` and `STR:`; centered shows `RX:128` and `STR:+0000`.
- `Up:MM:SS` advances.
- `TX:` and `PKT:` both show 0 (XBee TX is not enabled yet).

If the page flickers heavily: render is taking too long and starving the poll loop. Profile by temporarily printing `millis()` deltas — but 5 Hz with `snprintf` on AVR should be fine.

- [ ] **Step 4: Commit**

```bash
git add shared/Protocol.h controller-firmware/src/main.cpp
git commit -m "controller: assemble ControlPacket and render diagnostic page at 5Hz"
```

---

## Task 8: Add 20 Hz XBee TX with rolling-window rate (stage 6)

**Files:**
- Modify: `controller-firmware/src/main.cpp`

- [ ] **Step 1: Add the TX cadence and rate-window bookkeeping**

Full replacement of `controller-firmware/src/main.cpp`:

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware super-loop. Stage 6 build: + 20Hz XBee TX with
 *        1-second rolling rate reported to the diagnostic view.
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static constexpr uint32_t kPollPeriodMs = 20;     // 50 Hz
static constexpr uint32_t kTxPeriodMs = 50;       // 20 Hz
static constexpr uint32_t kRenderPeriodMs = 200;  // 5 Hz
static constexpr uint32_t kRateWindowMs = 1000;   // 1 s rolling window

static ControlPacket g_lastTx = {500, 0, 1, 0, 0};
static uint32_t g_txCount = 0;
static uint16_t g_txRateHz = 0;

static void scanI2c() {
  Serial.println(F("I2C scan begin"));
  uint8_t found = 0;
  for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  found 0x"));
      Serial.println(addr, HEX);
      found++;
    }
  }
  Serial.print(F("I2C scan end: "));
  Serial.print(found);
  Serial.println(F(" device(s)"));
}

static void buildControlPacket() {
  g_lastTx.throttle = ps4.throttle();
  g_lastTx.steering = ps4.steering();
  g_lastTx.mode = 1;
  g_lastTx.flags = ps4.failsafeActive() ? FAILSAFE_BIT : 0;
  g_lastTx.checksum = 0;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600);
  ps4.begin();
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 2000) {}
  scanI2c();
  if (!view.begin()) {
    Serial.println(F("OLED init FAILED"));
  } else {
    view.showBoot("boot stage 6");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastPoll = 0;
  static uint32_t lastTx = 0;
  static uint32_t lastRender = 0;
  static uint32_t lastRateMark = 0;
  static uint32_t txCountAtMark = 0;

  if (now - lastPoll >= kPollPeriodMs) {
    lastPoll = now;
    ps4.poll();
    buildControlPacket();
  }

  if (now - lastTx >= kTxPeriodMs) {
    lastTx = now;
    Serial1.write(reinterpret_cast<const uint8_t*>(&g_lastTx),
                  sizeof(ControlPacket));
    g_txCount++;
  }

  if (now - lastRateMark >= kRateWindowMs) {
    g_txRateHz = static_cast<uint16_t>(g_txCount - txCountAtMark);
    txCountAtMark = g_txCount;
    lastRateMark = now;
  }

  if (now - lastRender >= kRenderPeriodMs) {
    lastRender = now;
    view.render(ps4, g_lastTx, g_txCount, g_txRateHz, now);
  }
}
```

- [ ] **Step 2: Build, flash, observe (spec §9 stage 6)**

```
pio run -d controller-firmware --target upload
```

OLED pass criteria:
- `TX:20` (±1) appears within ~2 s of boot.
- `PKT:` increments steadily.

Wire-side pass criteria — use **one** of:

(a) **Logic analyzer on the Pro Micro TX1 pin (pin 1):**
- 57600 8N1, expect a burst of `sizeof(ControlPacket) = 10` bytes every 50 ms.
- Verify exact byte count per burst equals 10 — that's `2+2+1+1+4`.

(b) **Second serial monitor on the XBee's UART side** (e.g. an FTDI on the XBee receive line at 57600):
- ~20 frames/sec, each 10 bytes, with the first two bytes (`throttle` little-endian) tracking the stick.

If `TX:` is 0 but `PKT:` is incrementing: rate-window math wrong — re-read step 1.
If `TX:` is hovering around 10 instead of 20: the OLED render is blocking long enough to skew the cadence — reduce render cost or move it off the critical path. (Acceptable for stage 6; revisit if it persists.)

- [ ] **Step 3: Commit**

```bash
git add controller-firmware/src/main.cpp
git commit -m "controller: send ControlPacket at 20Hz over XBee with rolling-rate display"
```

---

## Task 9: Verify failsafe behavior (stage 7)

**Files:** none (verification only — no code changes; the failsafe is already implemented in `PS4Interface` from Task 2 and consumed in Task 7's `buildControlPacket()`).

- [ ] **Step 1: Set up to observe TX bytes during disconnect**

Have a logic analyzer or second serial monitor on the XBee TX line as in Task 8 step 2.

- [ ] **Step 2: With controller connected, capture a baseline frame**

Confirm a typical frame with sticks centered:
- `throttle` bytes = `0xF4 0x01` (= 500 little-endian)
- `steering` bytes = `0x00 0x00`
- `mode` = `0x01`
- `flags` = `0x00`

- [ ] **Step 3: Power-down the DS4 (hold PS button ~10 s)**

Within ~200 ms of the last successful poll, expect:
- OLED line 0 flips to `PS4:--`.
- Mapped line shows `THR:0500  STR:+0000` regardless of last stick position.
- TX-side `flags` byte becomes `0x01`.

Pass criteria: all three of the above happen within one render period (~200 ms) of disconnect.

If `flags` does not flip: re-check `PS4Interface::connected()` — it must return false once `kStaleMs` has elapsed even if `_lastStatus` is still `PS4_OK` from before. Trace by adding a temporary `Serial.println(ps4.connected());` to the trace block.

- [ ] **Step 4: Re-pair and confirm recovery**

Power the DS4 back on; within ~2 s of it going solid, the OLED returns to `PS4:OK` and `flags` returns to `0x00`. Stick motion immediately drives THR/STR again.

- [ ] **Step 5: No commit — verification only**

If a behavioral fix was required, commit it with a message like `controller: fix failsafe staleness window` and re-run steps 3–4.

---

## Task 10: Remove debug scaffolding and pass clang-tidy

**Files:**
- Modify: `controller-firmware/src/main.cpp`

The I2C scan and the 5 Hz USB-serial trace were diagnostic aids for stages 3–6. With stage 7 passing, they no longer earn their bytes. The boot banner stays as a power-on indicator.

- [ ] **Step 1: Final `main.cpp`**

Full replacement of `controller-firmware/src/main.cpp`:

```cpp
/**
 * @file main.cpp
 * @brief Controller firmware: 50Hz PS4 poll, 20Hz XBee TX, 5Hz OLED render.
 *        Failsafes to centered throttle / zero steering when the controller
 *        link goes stale (see PS4Interface::connected()).
 */

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticView.h"
#include "PS4Interface.h"
#include "Protocol.h"

PS4Interface ps4;
DiagnosticView view;

static constexpr uint32_t kPollPeriodMs = 20;     // 50 Hz
static constexpr uint32_t kTxPeriodMs = 50;       // 20 Hz
static constexpr uint32_t kRenderPeriodMs = 200;  // 5 Hz
static constexpr uint32_t kRateWindowMs = 1000;

static ControlPacket g_lastTx = {500, 0, 1, 0, 0};
static uint32_t g_txCount = 0;
static uint16_t g_txRateHz = 0;

static void buildControlPacket() {
  g_lastTx.throttle = ps4.throttle();
  g_lastTx.steering = ps4.steering();
  g_lastTx.mode = 1;
  g_lastTx.flags = ps4.failsafeActive() ? FAILSAFE_BIT : 0;
  g_lastTx.checksum = 0;
}

void setup() {
  Serial1.begin(57600);
  ps4.begin();
  if (view.begin()) {
    view.showBoot("v0.1 ready");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastPoll = 0;
  static uint32_t lastTx = 0;
  static uint32_t lastRender = 0;
  static uint32_t lastRateMark = 0;
  static uint32_t txCountAtMark = 0;

  if (now - lastPoll >= kPollPeriodMs) {
    lastPoll = now;
    ps4.poll();
    buildControlPacket();
  }

  if (now - lastTx >= kTxPeriodMs) {
    lastTx = now;
    Serial1.write(reinterpret_cast<const uint8_t*>(&g_lastTx),
                  sizeof(ControlPacket));
    g_txCount++;
  }

  if (now - lastRateMark >= kRateWindowMs) {
    g_txRateHz = static_cast<uint16_t>(g_txCount - txCountAtMark);
    txCountAtMark = g_txCount;
    lastRateMark = now;
  }

  if (now - lastRender >= kRenderPeriodMs) {
    lastRender = now;
    view.render(ps4, g_lastTx, g_txCount, g_txRateHz, now);
  }
}
```

USB `Serial` is intentionally not started — the Pro Micro's USB stack still enumerates as a CDC device on plug-in (the ATmega32U4 USB hardware handles that), but no application traffic is sent. If at any point we need USB serial again, add `Serial.begin(115200);` back at the top of `setup()`.

- [ ] **Step 2: Build**

Run: `pio run -d controller-firmware`
Expected: clean build, no warnings about unused functions.

- [ ] **Step 3: Run clang-tidy**

Run: `pio check -d controller-firmware --fail-on-defect high`
Expected: exits 0. If it flags issues:
- `modernize-use-nullptr`, `modernize-use-default-member-init` — apply the trivial fix.
- Warnings in the upstream `Gamepad_PS4BT` source are not ours to fix; if they show as `high`, scope check_flags to exclude the library: that's a separate cleanup, not part of this plan.

- [ ] **Step 4: Re-flash and re-confirm stages 5–7 still pass**

Quick sanity loop after the cleanup:
- OLED renders the diagnostic page.
- TX byte stream still 20 Hz with correct frame size.
- Disconnect → `PS4:--` and `flags=0x01` within 200 ms.

- [ ] **Step 5: Commit**

```bash
git add controller-firmware/src/main.cpp
git commit -m "controller: remove I2C scan and USB-serial trace scaffolding"
```

---

## Done criteria

The bring-up is complete when:

1. `pio run -d controller-firmware` is clean.
2. `pio check -d controller-firmware --fail-on-defect high` is clean.
3. With a paired DS4: OLED matches spec §6, TX:20 Hz, PKT increments, stick motion → THR/STR motion.
4. Power-down DS4: OLED `PS4:--`, mapped THR=500/STR=0, TX `flags` byte = 0x01, all within one render period.
5. Power-up DS4 again: full recovery within ~2 s.

At that point, hand off the next development phase: rover-side firmware to receive these `ControlPacket`s, plus the deferred Phase-1 "direction control" protocol work needed to make reverse throttle actually reach the wheels.
