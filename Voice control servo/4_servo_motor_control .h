#pragma once
#include <gpiod.h>
#include <string>

class Servo {
public:
    Servo(int gpio_pin, const std::string& label = "servo");
    ~Servo();

    void rotateTo(int angle);         // instant rotation
    void smoothRotateTo(int angle);   // smooth transition
    void release();

private:
    int pin;
    std::string label;
    gpiod_chip* chip;
    gpiod_line* line;

    void generatePWM(int pulse_width_us, int duration_ms);
};
