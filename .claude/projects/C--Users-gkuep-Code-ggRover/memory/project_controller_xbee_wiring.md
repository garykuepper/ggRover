---
name: project-controller-xbee-wiring
description: XBee on the controller must be wired to Pro Micro pins 0 (RX1) and 1 (TX1) — Serial1 hardware UART — not pins 8/9
metadata:
  type: project
---

XBee on the controller side connects to the Pro Micro's hardware UART: Pro Micro pin 0 = RX1 (from XBee TX), pin 1 = TX1 (to XBee RX). Baud 57600.

**Why:** As of 2026-05-23 the user originally had the XBee on pins 8/9 (digital, would have required SoftwareSerial). They re-wired to 0/1 so the controller firmware can use the rock-solid hardware UART (`Serial1`) at 57600 while the 50 Hz I2C poll runs concurrently. SoftwareSerial at that baud on a 16 MHz AVR is known to drop bytes when `Wire` interrupts fire.

**How to apply:** All controller firmware that talks to the XBee should use `Serial1`. If a future change ever needs to fall back to SoftwareSerial, drop the baud to ≤19200 and expect occasional dropped frames during concurrent I2C activity. See [[project-controller-bringup]] for the current bring-up plan.
