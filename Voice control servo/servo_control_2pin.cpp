#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>
#include "rotate180.h"

#define GPIO_CHIP   "/dev/gpiochip0"
#define SERVO_PIN   13  // 伺服控制口：GPIO 13
#define SERVO_PIN2  12  // 伺服控制口：GPIO 12

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
 * @brief 内部公共函数：控制单个伺服电机平滑旋转到 180°
 *
 * 对于所有伺服电机均采用相同的 PWM 控制曲线：
 * 从 MIN_PULSE_US 平滑增加到 MAX_PULSE_US，保证 0° 到 180° 的转动一致。
 *
 * @param servo_pin 指定的 GPIO 管脚号
 */
void rotateServo180_common(int servo_pin) {
    // 打开 GPIO 芯片
    gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Unable to open GPIO chip " << GPIO_CHIP << std::endl;
        return;
    }
    // 获取对应的 GPIO 线路
    gpiod_line* line = gpiod_chip_get_line(chip, servo_pin);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << servo_pin << std::endl;
        gpiod_chip_close(chip);
        return;
    }
    // 请求该 GPIO 线路为输出模式，初始状态为低电平
    if (gpiod_line_request_output(line, "rotate180", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return;
    }
   
    std::cout << "[rotateServo180] Starting smooth rotation from 0° to 180° (GPIO " << servo_pin << ")..." << std::endl;
   
    const int step_duration_ms = 50; // 每步输出50ms
    const int steps = 20;            // 分为20步平滑增加脉宽
    int step_delta  = (MAX_PULSE_US - MIN_PULSE_US) / steps;
    int pulse_width = MIN_PULSE_US;
   
    // 逐步增加脉宽，实现平滑旋转
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width += step_delta;
    }
   
    std::cout << "[rotateServo180] Servo on GPIO " << servo_pin << " reached 180°, holding position." << std::endl;
    // 在180°位置保持一段时间
    generate_pwm(line, MAX_PULSE_US, PWM_PERIOD_US, 1000);
   
    // 停止 PWM 输出并释放资源
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
   
    std::cout << "[rotateServo180] Rotation complete (GPIO " << servo_pin << ")." << std::endl;
}

/**
 * @brief 同时控制两个伺服电机（GPIO13 与 GPIO12）转动至 180°
 *
 * 采用相同的 PWM 脉宽计算策略，保证两个舵机转动效果一致。
 */
void rotateServo180() {
    std::thread servo1(rotateServo180_common, SERVO_PIN);
    std::thread servo2(rotateServo180_common, SERVO_PIN2);
    servo1.join();
    servo2.join();
}
