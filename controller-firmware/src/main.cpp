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
