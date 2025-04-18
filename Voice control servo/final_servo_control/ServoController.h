#pragma once
#include "Servo.h"

class ServoController {
public:
    ServoController();
    ~ServoController();

    void standUp();
    void sleep();
    void moveForward();
    void alternate();

private:
    Servo left;
    Servo right;
    void displayStatus(const std::string& status); // new OLED helper
};
