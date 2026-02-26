#include "HeadingSystem.h"

HeadingSystem::HeadingSystem() {}

bool HeadingSystem::begin() {
  if (_imu.begin() != INV_SUCCESS) {
    return false;
  }
  _imu.dmpBegin(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_GYRO_CAL, 10);
  return true;
}

void HeadingSystem::update() {
  if (_imu.fifoAvailable()) {
    _imu.dmpUpdateFifo();
    _imu.computeEulerAngles();
  }
}

float HeadingSystem::getYaw() { return _imu.yaw; }
float HeadingSystem::getPitch() { return _imu.pitch; }
float HeadingSystem::getRoll() { return _imu.roll; }
