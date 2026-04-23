# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

All commands are run from the relevant subdirectory (`rover-firmware/` or `controller-firmware/`).

```bash
# Build rover firmware
pio run -d rover-firmware

# Build controller firmware
pio run -d controller-firmware

# Static analysis (clang-tidy) — fails on high-severity defects
pio check -d rover-firmware --fail-on-defect high
pio check -d controller-firmware --fail-on-defect high

# Clean build artifacts
pio run -d rover-firmware --target clean
```

There is no test suite; hardware validation is done on-device.

## Architecture

This is a **dual-firmware monorepo** targeting two separate MCUs that communicate over an XBee serial radio link:

- **`rover-firmware/`** — STM32F103C8 ("Blue Pill", ARM Cortex-M3). Runs motor control, reads all sensors, and sends telemetry at 10Hz.
- **`controller-firmware/`** — Arduino Pro Micro (ATmega32U4, 8MHz). Reads the PS4 gamepad via I2C (Hobbytronics USB Host adapter) and drives an SSD1306 OLED dashboard. Sends commands at 20Hz.
- **`shared/`** — Header-only protocol included by both firmwares via `-I ../shared` in each `platformio.ini`.

### Communication Protocol (`shared/Protocol.h`)

Two `#pragma pack(1)` structs ensure identical byte layout on both the 8-bit AVR and 32-bit STM32:

- **`ControlPacket`** (Controller → Rover): throttle (0–1000), steering (−500–500), mode, flags, CRC32.
- **`TelemetryPacket`** (Rover → Controller): battery voltage, IMU roll/pitch/yaw, encoder counts, temperature, pressure, ToF distance, CRC32.

Any change to these structs must maintain alignment compatibility across both architectures.

### Rover Class Responsibilities

| Class | Responsibility |
|---|---|
| `Drivetrain` | PWM output to 4 motors (FL/FR/RL/RR); simple throttle+steering mixing |
| `HeadingSystem` | MPU-9250 DMP at 10Hz; produces stable Euler angles via FIFO |
| `Odometry` | STM32 hardware-timer encoder counting — **currently incomplete** |

### Controller Class Responsibilities

| Class | Responsibility |
|---|---|
| `PS4Interface` | I2C reads from Hobbytronics USB Host (addr 0x29); maps 8-byte report to throttle/steering — **currently skeleton** |
| `Dashboard` | SSD1306 128×64 OLED; renders battery, yaw, ToF distance, temperature from incoming telemetry |

## Code Style

- Google C++ Style enforced by `.clang-format` (2-space indent, 100-column limit).
- `PascalCase` for classes, `camelCase` for methods and variables.
- clang-tidy checks: `bugprone-*`, `modernize-*`, `readability-*`, `performance-*`.
