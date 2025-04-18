#pragma once
#include "Servo.h"

class ServoController {
public:
    ServoController();
    ~ServoController();

    void standUp();         // both to 180°
    void sleep();           // both to 0°
    void moveForward();     // both to 90°
    void alternate();       // forward/backward alternating motion

private:
    Servo left;   // GPIO 13
    Servo right;  // GPIO 12
};
