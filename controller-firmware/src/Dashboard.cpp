#include "Dashboard.h"

Dashboard::Dashboard(uint8_t width, uint8_t height)
    : _display(width, height, &Wire, -1) {}

bool Dashboard::begin() {
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    return false;
  }
  _display.clearDisplay();
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.display();
  return true;
}

void Dashboard::update(const TelemetryPacket& data) {
  _display.clearDisplay();
  _display.setCursor(0, 0);
  _display.print("V: "); _display.print(data.batteryVoltage); _display.println("V");
  _display.print("Y: "); _display.println(data.yaw);
  _display.print("D: "); _display.print(data.distance); _display.println("mm");
  _display.print("T: "); _display.print(data.temperature); _display.println("C");
  _display.display();
}

void Dashboard::showMessage(const char* msg) {
  _display.clearDisplay();
  _display.setCursor(0, 10);
  _display.println(msg);
  _display.display();
}
