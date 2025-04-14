#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>
#include "rotate180.h"

#define GPIO_CHIP   "/dev/gpiochip0"
#define SERVO_PIN   13  // 舵机控制口：GPIO 13
#define SERVO_PIN2  12  // 舵机控制口：GPIO 12

// PWM 参数设置
const int PWM_PERIOD_US = 20000;  // 20ms 周期（50Hz）
const int MIN_PULSE_US   = 1000;   // 1ms 脉宽对应 0°
const int MAX_PULSE_US   = 2000;   // 2ms 脉宽对应 180°

/**
 * @brief 输出指定脉宽的 PWM 信号，持续一段时间（单位：毫秒）
 *
 * @param line 控制的 GPIO 线路
 * @param pulse_width_us 脉宽（单位：微秒）
 * @param period_us PWM 周期（单位：微秒）
 * @param duration_ms 输出持续时间（单位：毫秒）
 */
void generate_pwm(gpiod_line* line, int pulse_width_us, int period_us, int duration_ms) {
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time).count() < duration_ms) {
        if (gpiod_line_set_value(line, 1) < 0) {
            std::cerr << "Failed to set high level" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_width_us));
        if (gpiod_line_set_value(line, 0) < 0) {
            std::cerr << "Failed to set low level" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(period_us - pulse_width_us));
    }
}

/**
 * @brief 内部公共函数：控制单个伺服电机平滑旋转从 0° 到 180°
 *
 * @param servo_pin 指定的 GPIO 管脚号
 */
void rotateServo180_common(int servo_pin) {
    gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Unable to open GPIO chip " << GPIO_CHIP << std::endl;
        return;
    }
    gpiod_line* line = gpiod_chip_get_line(chip, servo_pin);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << servo_pin << std::endl;
        gpiod_chip_close(chip);
        return;
    }
    if (gpiod_line_request_output(line, "rotate180", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return;
    }

    std::cout << "[rotateServo180] Starting smooth rotation from 0° to 180° (GPIO " << servo_pin << ")..." << std::endl;
    const int step_duration_ms = 50;
    const int steps = 20;
    int step_delta  = (MAX_PULSE_US - MIN_PULSE_US) / steps;
    int pulse_width = MIN_PULSE_US;
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width += step_delta;
    }
    std::cout << "[rotateServo180] Servo on GPIO " << servo_pin << " reached 180°, holding position." << std::endl;
    generate_pwm(line, MAX_PULSE_US, PWM_PERIOD_US, 1000);
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    std::cout << "[rotateServo180] Rotation complete (GPIO " << servo_pin << ")." << std::endl;
}

/**
 * @brief 内部公共函数：控制单个伺服电机平滑旋转从 180° 到 0°
 *
 * 用于 sleep 模式下的舵机归位。
 *
 * @param servo_pin 指定的 GPIO 管脚号
 */
void rotateServo0_common(int servo_pin) {
    gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Unable to open GPIO chip " << GPIO_CHIP << std::endl;
        return;
    }
    gpiod_line* line = gpiod_chip_get_line(chip, servo_pin);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << servo_pin << std::endl;
        gpiod_chip_close(chip);
        return;
    }
    if (gpiod_line_request_output(line, "rotate180", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return;
    }

    std::cout << "[rotateServo0] Starting smooth rotation from 180° to 0° (GPIO " << servo_pin << ")..." << std::endl;
    const int step_duration_ms = 50;
    const int steps = 20;
    int step_delta = (MAX_PULSE_US - MIN_PULSE_US) / steps;
    int pulse_width = MAX_PULSE_US;
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width -= step_delta;
        if (pulse_width < MIN_PULSE_US)
            pulse_width = MIN_PULSE_US;
    }
    std::cout << "[rotateServo0] Servo on GPIO " << servo_pin << " reached 0°, holding position." << std::endl;
    generate_pwm(line, MIN_PULSE_US, PWM_PERIOD_US, 1000);
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    std::cout << "[rotateServo0] Rotation complete (GPIO " << servo_pin << ")." << std::endl;
}

/**
 * @brief 内部公共函数：控制单个舵机平滑旋转到 90° (move forward 模式)
 *
 * 对于 GPIO13 的舵机：从 0° (1000us) 到 90° (1500us)；
 * 对于 GPIO12 的舵机：从 180° (2000us) 到 90° (1500us)。
 *
 * @param servo_pin 指定的 GPIO 管脚号
 */
void moveForward_common(int servo_pin) {
    gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Unable to open GPIO chip " << GPIO_CHIP << std::endl;
        return;
    }
    gpiod_line* line = gpiod_chip_get_line(chip, servo_pin);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << servo_pin << std::endl;
        gpiod_chip_close(chip);
        return;
    }
    if (gpiod_line_request_output(line, "moveForward", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return;
    }

    int start_pulse, target_pulse;
    target_pulse = (MIN_PULSE_US + MAX_PULSE_US) / 2;  // 1500us 表示90°
    if (servo_pin == SERVO_PIN) {
        start_pulse = MIN_PULSE_US;
        std::cout << "[moveForward] Servo on GPIO " << servo_pin << " moving from 0° to 90°..." << std::endl;
    } else if (servo_pin == SERVO_PIN2) {
        start_pulse = MAX_PULSE_US;
        std::cout << "[moveForward] Servo on GPIO " << servo_pin << " moving from 180° to 90°..." << std::endl;
    } else {
        gpiod_chip_close(chip);
        return;
    }

    const int step_duration_ms = 50;
    const int steps = 10;
    int pulse_diff = target_pulse - start_pulse;
    int step_delta = pulse_diff / steps;
    int pulse_width = start_pulse;
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width += step_delta;
        if (step_delta > 0 && pulse_width > target_pulse)
            pulse_width = target_pulse;
        else if (step_delta < 0 && pulse_width < target_pulse)
            pulse_width = target_pulse;
    }
    generate_pwm(line, target_pulse, PWM_PERIOD_US, 1000);
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    std::cout << "[moveForward] Servo on GPIO " << servo_pin << " reached 90°." << std::endl;
}

/**
 * @brief 同时控制两个伺服电机实现 move forward 操作：
 *        GPIO13 的舵机平滑从 0° 旋转到 90°；
 *        GPIO12 的舵机平滑从 180° 旋转到 90°。
 */
void moveForward() {
    std::thread servo1(moveForward_common, SERVO_PIN);
    std::thread servo2(moveForward_common, SERVO_PIN2);
    servo1.join();
    servo2.join();
}

/**
 * @brief 同时控制两个伺服电机（GPIO13 与 GPIO12）旋转至 180°
 */
void rotateServo180() {
    std::thread servo1(rotateServo180_common, SERVO_PIN);
    std::thread servo2(rotateServo180_common, SERVO_PIN2);
    servo1.join();
    servo2.join();
}

/**
 * @brief 同时控制两个伺服电机（GPIO13 与 GPIO12）旋转回 0° (sleep 模式)
 */
void rotateServo0() {
    std::thread servo1(rotateServo0_common, SERVO_PIN);
    std::thread servo2(rotateServo0_common, SERVO_PIN2);
    servo1.join();
    servo2.join();
}

/**
 * @brief 交替执行舵机运动：
 *        先使 GPIO12 舵机从 180° 平滑旋转到 90°（利用 moveForward_common 实现），
 *        再使 GPIO13 舵机从 0° 平滑旋转到 180°，
 *        共交替运行 6 个周期。
 */
void alternateRotation() {
    for (int i = 0; i < 6; i++) {
        std::cout << "[alternateRotation] Cycle " << (i + 1)
                  << " : GPIO12 rotates forward (from 180° to 90°)" << std::endl;
        // GPIO12：从 180° 到 90°采用 moveForward_common（仅一次动作）
        moveForward_common(SERVO_PIN2);
        std::cout << "[alternateRotation] Cycle " << (i + 1)
                  << " : GPIO13 rotates backward (from 0° to 180°)" << std::endl;
        rotateServo180_common(SERVO_PIN);
    }
}
