# ggRover Development Roadmap

This document outlines the phased development strategy for the Semi-Autonomous Rover System.

## Phase 1: Hardware Validation & Low-Level Control
**Goal**: Establish reliable low-level control of basic hardware.

- [ ] **Rover**: Implement STM32 Hardware Timer encoder counting in `Odometry` class.
- [ ] **Rover**: Calibrate `Drivetrain` motor mixing and PWM ranges.
- [ ] **Controller**: Map I2C gamepad reports from Hobbytronics USB Host into `PS4Interface`.
- [ ] **Controller**: Design basic telemetry layout for SSD1306 `Dashboard`.

## Phase 2: Communication & Safety
**Goal**: Robust binary link and failsafe mechanisms.

- [ ] **Shared**: Verify `Protocol.h` struct alignment between 8-bit (Pro Micro) and 32-bit (STM32) architectures.
- [ ] **System**: Establish XBee Serial link with 20Hz command / 10Hz telemetry loops.
- [ ] **Safety**: Implement a "Dead-man's Switch" failsafe (Rover stops if no packets received for >500ms).
- [ ] **Safety**: Add battery voltage monitoring and low-voltage cutoff warnings on the Controller.

## Phase 3: Sensor Fusion & Environment
**Goal**: Integrate spatial awareness and environmental monitoring.

- [ ] **Rover**: Finalize `HeadingSystem` with DMP (Digital Motion Processor) for stable yaw/heading.
- [ ] **Rover**: Implement collision avoidance logic using the VL53L0X ToF sensor data.
- [ ] **Controller**: Add graphing/visual indicators for BME280 temperature and pressure data.

## Phase 4: Autonomy & Advanced Features
**Goal**: Move beyond manual remote control.

- [ ] **Rover**: Implement PID-based heading hold (auto-straight drive using IMU feedback).
- [ ] **Rover**: Implement waypoint navigation using encoder-based odometry.
- [ ] **System**: Add a "Record & Replay" feature to store and repeat paths.
