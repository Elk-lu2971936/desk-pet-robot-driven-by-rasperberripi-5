#include "ServoController.h"
#include <thread>
#include <iostream>

ServoController::ServoController()
    : left(13, "servo13"), right(12, "servo12") {}

ServoController::~ServoController() {
    left.release();
    right.release();
}

void ServoController::standUp() {
    std::thread t1([&]() { left.smoothRotateTo(180); });
    std::thread t2([&]() { right.smoothRotateTo(180); });
    t1.join(); t2.join();
}

void ServoController::sleep() {
    std::thread t1([&]() { left.smoothRotateTo(0); });
    std::thread t2([&]() { right.smoothRotateTo(0); });
    t1.join(); t2.join();
}

void ServoController::moveForward() {
    std::thread t1([&]() { left.smoothRotateTo(90); });
    std::thread t2([&]() { right.smoothRotateTo(90); });
    t1.join(); t2.join();
}

void ServoController::alternate() {
    for (int i = 0; i < 6; ++i) {
        std::cout << "[Cycle " << i + 1 << "] GPIO12 -> 90°, GPIO13 -> 180°" << std::endl;
        right.smoothRotateTo(90);
        left.smoothRotateTo(180);
    }
}
