# ggRover: Semi-Autonomous Rover System

A modular, object-oriented C++ monorepo for a dual-hardware rover system using PlatformIO.

## Project Goals
- **Modular Control**: Establish a clean separation between the Rover's low-level hardware control and the Controller's user interface.
- **Semi-Autonomous Operation**: Support manual control via PS4 Bluetooth gamepad with real-time telemetry feedback (IMU, ToF, Environment).
- **Extensible Architecture**: Provide a robust C++ foundation for adding autonomous navigation and advanced sensor fusion.

## System Architecture

### 1. Rover (STM32 Blue Pill)
The core vehicle controller responsible for motor orchestration and sensor data collection.
- **Hardware**: STM32F103C8, 4WD DC Motors with Encoders.
- **Sensors**: MPU-9250 (IMU), VL53L0X (ToF), BME280 (Environmental).
- **Communication**: XBee (Serial) for remote link.

### 2. Remote Controller (Arduino Pro Micro)
The handheld interface for pilot control and telemetry visualization.
- **Hardware**: Arduino Pro Micro (3.3V).
- **Interface**: Hobbytronics USB Host (I2C) for PS4 DualShock 4 support, SSD1306 OLED for feedback.
- **Communication**: XBee (Serial) for rover link.

## Project Structure
```text
/ggRover
├── shared/                  # Binary-safe communication protocols
│   └── Protocol.h           # Shared Control/Telemetry packets
├── rover-firmware/          # STM32 low-level firmware
│   ├── include/             # Class headers (Drivetrain, Odometry, etc.)
│   └── src/                 # Implementation and main loop
└── controller-firmware/     # Pro Micro remote firmware
    ├── include/             # Dashboard and Gamepad interfaces
    └── src/                 # Implementation and main loop
```

## Engineering Standards
- **Formatting**: Google C++ Style (2-space indent) enforced via `.clang-format`.
- **Naming**: `PascalCase` for Classes, `camelCase` for methods/variables.
- **Build System**: PlatformIO with strictly defined `platformio.ini` environments.
