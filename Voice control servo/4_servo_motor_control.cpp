#include "Servo.h"
#include <iostream>
#include <thread>
#include <chrono>

#define GPIO_CHIP   "/dev/gpiochip0"
#define PWM_PERIOD_US 20000
#define MIN_PULSE_US 1000
#define MAX_PULSE_US 2000

Servo::Servo(int gpio_pin, const std::string& label)
    : pin(gpio_pin), label(label), chip(nullptr), line(nullptr) {
    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) throw std::runtime_error("Failed to open GPIO chip");
    line = gpiod_chip_get_line(chip, pin);
    if (!line) throw std::runtime_error("Failed to get GPIO line");
    if (gpiod_line_request_output(line, label.c_str(), 0) < 0)
        throw std::runtime_error("Failed to request GPIO output");
}

Servo::~Servo() {
    if (line) gpiod_line_release(line);
    if (chip) gpiod_chip_close(chip);
}

void Servo::generatePWM(int pulse_width_us, int duration_ms) {
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start).count() < duration_ms) {
        gpiod_line_set_value(line, 1);
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_width_us));
        gpiod_line_set_value(line, 0);
        std::this_thread::sleep_for(std::chrono::microseconds(PWM_PERIOD_US - pulse_width_us));
    }
}

void Servo::rotateTo(int angle) {
    int pulse = MIN_PULSE_US + (angle * (MAX_PULSE_US - MIN_PULSE_US) / 180);
    generatePWM(pulse, 1000);
    gpiod_line_set_value(line, 0);
}

void Servo::smoothRotateTo(int target_angle) {
    int start_pulse = MIN_PULSE_US;
    int target_pulse = MIN_PULSE_US + (target_angle * (MAX_PULSE_US - MIN_PULSE_US) / 180);
    int steps = 20;
    int step_duration_ms = 50;
    int step = (target_pulse - start_pulse) / steps;

    for (int i = 0; i <= steps; i++) {
        generatePWM(start_pulse + i * step, step_duration_ms);
    }
    generatePWM(target_pulse, 500);
    gpiod_line_set_value(line, 0);
}

void Servo::release() {
    gpiod_line_set_value(line, 0);
}
