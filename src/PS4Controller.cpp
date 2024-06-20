#include "PS4Controller.h"

PS4Controller::PS4Controller() {}

void PS4Controller::begin() {
    _ps4.begin();
}

void PS4Controller::update() {
    _ps4.update();
}

int PS4Controller::getLX() {
    return _ps4.getLX();
}

int PS4Controller::getLY() {
    return _ps4.getLY();
}

int PS4Controller::getRX() {
    return _ps4.getRX();
}

int PS4Controller::getRY() {
    return _ps4.getRY();
}
