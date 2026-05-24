# ggRover — Hardware Specification Sheet

Source of truth for **what hardware exists, what it's rated for, and how it's configured**. Pair with [docs/pinouts.md](docs/pinouts.md) (which is the source of truth for *where each piece is wired*).

When the firmware and this doc disagree, **this doc wins** for hardware questions — the firmware is currently a skeleton and contains references to sensors that may not be on the final BOM. See [Firmware ↔ spec discrepancies](#firmware--spec-discrepancies) at the bottom.

---

## Microcontroller
| | |
|---|---|
| Board | STM32F103C8T6 Blue Pill |
| Clock | 72 MHz |
| Logic voltage | 3.3V |
| Encoder timers | 4× hardware (TIM1–TIM4) |
| PWM channels | 8 needed (4× motor + 4× servo) |
| I2C bus | 1× shared across all sensors |

---

## Drive Motors ×4
| | |
|---|---|
| Model | JGA25-370 |
| Voltage | 12V |
| No-load speed | 280 RPM (21.3:1 gearbox) |
| At-load speed | ~220 RPM |
| Stall current | 1.3A per motor |
| Output shaft | 4mm Ø with flat |
| Encoder type | AB Hall effect, 11 PPR |
| Encoder wires | 6-wire — M+, M−, VCC, GND, A, B |
| Counts per wheel rev | 11 × 4 × 21.3 = ~937 |
| Encoder voltage | 3.3V — Blue Pill direct ✓ |

---

## Motor Drivers ×4 — **open**
ESC/H-bridge choice not locked. Candidates and requirements:

| | |
|---|---|
| Required continuous current | ≥1.5 A per channel (1.15× motor stall) |
| Required supply | 14.4 V nominal, 16.8 V peak (4S battery direct) |
| Required logic | 3.3 V tolerant (Blue Pill direct, no level-shifter) |
| Candidate A | DRV8871 — 3.6 A, IN1/IN2 (PWM + direction, **8 GPIOs total**) |
| Candidate B | TB6612FNG dual — 1.2 A cont. (under-spec at stall), PWMA/AIN1/AIN2 per channel |
| Candidate C | Brushed micro-ESC (RC-style, 1-wire PWM) — simplest pin count but no regen braking |

> Pin budget impact: Candidate A doubles rover PWM/GPIO usage (4 → 8) and forces a TIM2 reshuffle (see [pinouts.md](pinouts.md)). Candidate C keeps the current 1-pin-per-motor scheme.

---

## Steering Servos ×4 — **model open**
| | |
|---|---|
| Count | 4 total — 1 per wheel (4WIS) |
| Candidate model | MG996R (rejected: MG90S — too weak) |
| Min torque target | ~9 kg·cm @ 4.8V |
| Signal | 50Hz PWM, 1–2ms pulse |
| Gears | Metal preferred |
| Modes | Ackermann / Crab / Spin in place / Straight |

---

## Wheels
| | |
|---|---|
| Diameter | 75mm |
| Width | 30mm |
| Hub | 12mm hex |
| Type | RC rally — foam inserts |
| Adapter needed | 4mm shaft → 12mm hex |
| Top speed (est.) | ~4 km/h @ 12V PWM |

---

## Suspension
| | |
|---|---|
| Type | Double wishbone, independent ×4 |
| Upper arm | 42mm |
| Lower arm | 60mm |
| Shock absorbers | 100mm |
| Travel | ~25mm |
| Ride height | 30mm |
| Joints | M3 pin bolts — no ball joints |
| Kingpin | M4 stainless bolt (steering axis) |
| Tie rod | ~55mm |
| Steering arm | ~22mm |

---

## Chassis
| | |
|---|---|
| Wheelbase | 220mm |
| Track (c-c) | 240mm |
| Width | 120mm |
| Length | 200mm |
| Material | 3D printed ABS |
| Battery bay | 144 × 65 × 36mm (L × W × H) |

---

## Battery
| | |
|---|---|
| Configuration | 4S4P — self-assembled |
| Cells | 16× 18650 |
| Nominal voltage | 14.4V |
| Max voltage | 16.8V fully charged |
| Pack dimensions | 144 × 65 × 36mm |
| BMS rating | 30A continuous |
| Motor PWM cap | ~71% duty = 12V equivalent |
| Logic supply | ⚠ Buck converter needed → 5V |
| Est. runtime | >3 hours |

---

## Sensors & Peripherals

**Baseline (required for v1):**

| | |
|---|---|
| IMU | MPU9250 — 9-axis with DMP, I2C (rover) |
| Current sensor | INA226 ×4 — per-motor, I2C (rover) |
| Display | 0.91" OLED 128×32 — I2C (controller) |
| Gamepad | PS4 via Hobbytronics PS4BT I2C adapter (controller) |

**Future / not baseline** — present in firmware as forward-compat scaffolding; do not treat as required for v1:

| | |
|---|---|
| ToF range | VL53L0X — for collision avoidance (Phase 3) |
| Environment | BME280 — temp/pressure (Phase 3) |

---

## I2C Bus Map

Rover bus (Blue Pill `PB6`/`PB7`):

| Device | Address |
|---|---|
| MPU9250 | 0x68 |
| INA226 #1 (FL) | 0x40 |
| INA226 #2 (FR) | 0x41 |
| INA226 #3 (RL) | 0x42 |
| INA226 #4 (RR) | 0x43 |
| VL53L0X (future) | 0x29 |
| BME280 (future) | 0x76 |

Controller bus (Pro Micro `D2`/`D3`):

| Device | Address |
|---|---|
| Hobbytronics PS4BT adapter | 0x29 |
| OLED 128×32 | 0x3C |

Pull-ups: 4.7 kΩ to logic-rail, one set per bus.

---

## Spare / Unused Hardware
| | |
|---|---|
| JGA25-370 1200RPM | 5× no encoder — spare |
| JGA25-370 280RPM encoder | 2× extra — spare |
| Brushless planetary motors | 4× — TBD |
| MG90S servos | Ordered — too weak for steering |
| Micro brushed ESCs | Various — not used |

---

## Outstanding Items
| Item | Status |
|---|---|
| 4mm → 12mm hex adapter | ⚠ Not confirmed ordered |
| Buck converter (16.8V → 5V) | ⚠ Not yet sourced |
| Brushless motors | ❓ Use case TBD |

> ⚠ Chassis dimensions are design targets — verify in OnShape before printing.

---

## Firmware ↔ spec discrepancies

Captured at the time this doc was written; remove rows as the firmware catches up.

**Controller firmware is locked-in (validated 2026-05-24)** — its hardware addresses (PS4 adapter `0x29`, OLED `0x3C`) and wiring are the source of truth. All discrepancies below are rover-side, to be resolved on the rover.

| Area | Spec says | Firmware does | Action |
| --- | --- | --- | --- |
| Drive output | 4 channels, control scheme depends on ESC choice | 1× `analogWrite` per wheel ([rover-firmware/src/Drivetrain.cpp:20-23](rover-firmware/src/Drivetrain.cpp#L20-L23)) — no reverse | **blocked on ESC selection.** If DRV8871 → rewrite for IN1/IN2 pairs + TIM2 reshuffle. If micro-ESC → current 1-PWM scheme is fine, just add direction calibration. |
| Servos | 4× steering, model TBD | no servo code at all | add `Steering` class; reserve 4 PWM pins (revisit if "4 per wheel" interpretation changes) |
| Per-motor current | INA226 ×4 at `0x40`–`0x43` | not initialized; not in `TelemetryPacket` | add driver + extend `TelemetryPacket` (protocol break — bump version) |
| Battery voltage | 14.4 V nominal / 16.8 V peak (4S4P confirmed) | hardcoded `12.6f` ([rover-firmware/src/main.cpp:66](rover-firmware/src/main.cpp#L66)) | wire ADC divider; update telemetry scale |
| Encoder CPR | 937 counts / wheel-rev | `getDistance()` placeholder `* 0.5f` ([rover-firmware/src/Odometry.cpp:26](rover-firmware/src/Odometry.cpp#L26)) | use real CPR + wheel circumference (75 mm Ø → ~235.6 mm/rev) |
| Logic 5 V rail | "buck converter needed — not yet sourced" | n/a | flagged in [Outstanding Items](#outstanding-items) — blocks bring-up |
| ToF / BME280 init | future scope (not baseline) | `Adafruit_VL53L0X` + `Adafruit_BME280` initialized & read every loop ([rover-firmware/src/main.cpp:25-26](rover-firmware/src/main.cpp#L25-L26)) | acceptable as forward-compat scaffolding; consider `#ifdef`-gating for v1 firmware footprint |
