# Controller Link Auto-Recovery — Design

**Date:** 2026-05-23
**Scope:** `controller-firmware/` + one hardware addition (RST wire to Hobbytronics).
**Status:** Draft, deferred. Bring-up plan is complete; this is the natural follow-on.

## 1. Goal

Make the controller self-recover when the DS4 link drops, without operator intervention. Today: failsafe engages correctly when the controller goes off-air, but the operator has to power-cycle the Hobbytronics adapter to get the link back. That's a non-starter for a remote-controlled rover.

## 2. Problem statement

Confirmed on bench (2026-05-23) using a Faraday-cage test (closed microwave around the DS4):

- After BT loss, the Hobbytronics adapter **does not surface the disconnect**. It keeps returning its last cached HID report with `PS4_OK` status indefinitely; its status LED also remains solid blue.
- Even after the controller powers back up, the adapter **does not re-pair** because it still believes a connection is live. It just sits there.
- The only known recovery without code changes is to physically remove and re-apply power to the Hobbytronics.

The bring-up plan's existing failsafe (`PS4Interface` accel-jitter heartbeat, commit `1c7d511`) correctly detects the dead link and neutralizes throttle/steering — that part already works. What's missing is the **un-stuck** path.

## 3. Hardware addition

The Hobbytronics 6-pin header is `VCC, GND, SDA, SCL, INT, ...` — confirmed no RST is broken out there. The board does have a **dedicated RST pad** elsewhere on the silkscreen.

Need:

- One thin wire soldered from the Hobbytronics RST pad to a free Pro Micro GPIO.
- Pull-up on the line is internal to the Hobbytronics; Pro Micro just needs to drive it low briefly.

Pin choice — open question for the operator. Reserved/used pins so far:

| Pin | Use |
|---|---|
| 0 (RX1), 1 (TX1) | Serial1 → XBee |
| 2 (SDA), 3 (SCL) | I2C → Hobbytronics + OLED |
| 5, 6, 9, 10 | PWM-capable — reserve for future motor PWM mirror / rumble feedback |

Suggested: **pin 4** — digital-only, far from any future PWM use, not consumed by anything in the current firmware.

## 4. Firmware state machine

`PS4Interface` already exposes `failsafeActive()` (the change-of-state we trigger on). The new module/method does:

```
states: NORMAL, RECOVERY_PULSE, RECOVERY_COOLDOWN

NORMAL:
  if (failsafeActive for >2000 ms) -> RECOVERY_PULSE

RECOVERY_PULSE:
  drive RST low
  wait 10 ms
  release RST (input or HIGH)
  -> RECOVERY_COOLDOWN, mark cooldown_start = millis()

RECOVERY_COOLDOWN:
  for next 3000 ms: do nothing (let Hobbytronics boot + re-pair)
  if (still failsafeActive after cooldown) -> RECOVERY_PULSE (back-off scheme TBD)
  if (PS4Interface.connected() returns true) -> NORMAL
```

Key constraints:

- **Don't pulse RST while the controller is paired.** Only act on a confirmed-dead link.
- **Don't pulse faster than once per ~3 sec.** Hobbytronics needs ~1.5–2 sec to boot and re-discover BT before we can judge.
- **Cap the retry rate** so a permanently-dead BT environment doesn't flap the adapter forever. Suggest 5 pulses then 30-sec quiet period.

## 5. Diagnostic UI

Add to `DiagnosticView`:

- New field `RST:n` on one of the existing 8 rows. `n` = pulse count since boot. Confirms the recovery path actually fires.
- Optionally a brief banner row swap during the `RECOVERY_PULSE` / `COOLDOWN` window: `LINK RECOVERING`.

## 6. Pass criteria

Operator-style smoke test on bench:

1. Pair DS4, confirm `PS4:OK` and `flags=0x00`.
2. Faraday-cage the DS4 (closed microwave, no actual cooking). Within ~200 ms: `flags=0x01`, throttle 500, steering 0. (Existing failsafe.)
3. **New:** within ~2.5 s of step 2, `RST:` increments by 1.
4. Pull the DS4 out of the cage and turn it back on. Within ~3–5 s: `PS4:OK`, `flags=0x00`, full control restored.
5. Steps 2-4 work repeatably across multiple cycles without manual intervention.

Failure modes to design around:

- DS4 left off / out of range permanently → RST pulses cap out, then quiet period. Recovers on next genuine controller appearance.
- Hobbytronics in a stuck state pre-existing at boot → first failsafe-driven pulse picks it up.

## 7. Open questions

- Pin choice (above).
- Active level of the Hobbytronics RST line — almost certainly active-low, but worth confirming from the silkscreen or by scope before wiring.
- Whether to surface "recovery in progress" in the `ControlPacket` flags (new bit) so the rover-side firmware can show it. Probably not for v1 — keep the protocol stable.

## 8. Out of scope (deferred further)

- CRC / framing on `ControlPacket` (still Phase 2 roadmap item).
- Rover-side analog of this same recovery (XBee link can also wedge; that's a separate spec because the symptoms are different — no equivalent of "cached report PS4_OK").
