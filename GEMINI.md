# Gemini Project Context: ggRover

A modular, object-oriented C++ monorepo for a dual-hardware rover system managed with PlatformIO.

## Project Overview

The **ggRover** project consists of two main firmware components that communicate over an XBee serial radio link using a custom binary-safe protocol.

-   **Rover (`rover-firmware/`)**: Targets an **STM32F103C8 (Blue Pill)**. Responsible for motor control (PWM), sensor data collection (MPU-9250 IMU, VL53L0X ToF, BME280 Environment), and broadcasting telemetry at 10Hz.
-   **Controller (`controller-firmware/`)**: Targets an **Arduino Pro Micro**. Interfaces with a PS4 DualShock 4 gamepad (via Hobbytronics USB Host adapter) and displays real-time telemetry on an SSD1306 OLED dashboard. Sends control commands at 20Hz.
-   **Shared (`shared/`)**: Contains `Protocol.h`, defining the `ControlPacket` and `TelemetryPacket` structs used by both firmwares.

## Architecture & Technology Stack

-   **Language**: C++ (Arduino framework)
-   **Build System**: PlatformIO
-   **MCU 1 (Rover)**: STM32F103C8 (ARM Cortex-M3)
-   **MCU 2 (Controller)**: ATmega32U4 (AVR, 8MHz)
-   **Communication**: XBee Serial (57600 baud), I2C for local peripherals.
-   **Key Libraries**:
    -   SparkFun MPU-9250 DMP (IMU)
    -   Adafruit VL53L0X (ToF)
    -   Adafruit BME280 (Environmental)
    -   Adafruit SSD1306 & GFX (OLED Display)

## Building and Running

Commands should be executed from the project root or the respective firmware directory.

### Build
```powershell
# Build Rover firmware
pio run -d rover-firmware

# Build Controller firmware
pio run -d controller-firmware
```

### Static Analysis
```powershell
# Run clang-tidy on Rover firmware
pio check -d rover-firmware --fail-on-defect high

# Run clang-tidy on Controller firmware
pio check -d controller-firmware --fail-on-defect high
```

### Upload
```powershell
# Upload to Rover (requires ST-Link or serial bootloader)
pio run -d rover-firmware --target upload

# Upload to Controller (Serial/USB)
pio run -d controller-firmware --target upload
```

## Development Conventions

-   **Code Style**: Google C++ Style (2-space indent, 100-column limit) enforced via `.clang-format`.
-   **Naming**:
    -   `PascalCase` for Classes (e.g., `Drivetrain`, `HeadingSystem`).
    -   `camelCase` for methods and variables (e.g., `getLeftTicks()`, `txPacket`).
    -   Snake_case for filenames (e.g., `main.cpp`).
-   **Communication**: Use `#pragma pack(1)` in `shared/Protocol.h` to ensure byte-alignment compatibility between the 8-bit AVR and 32-bit ARM architectures.
-   **Static Analysis**: `clang-tidy` checks include `bugprone-*`, `modernize-*`, `readability-*`, and `performance-*`.

## Current Status & Known Issues

-   **Odometry**: STM32 hardware timer initialization for encoders is currently a TODO in `rover-firmware/src/Odometry.cpp`.
-   **PS4Interface**: The I2C interface for the Hobbytronics USB Host is a skeleton and needs full implementation of the 8-byte report mapping.
-   **Telemetry**: Battery voltage in `rover-firmware/src/main.cpp` is currently a placeholder constant.
