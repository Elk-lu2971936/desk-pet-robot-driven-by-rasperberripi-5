#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>
#include "rotate180.h"

#define GPIO_CHIP   "/dev/gpiochip0"
#define SERVO_PIN   13  // 使用GPIO编号 13

// PWM 参数设置
const int PWM_PERIOD_US = 20000;  // 20ms 周期（50Hz）
const int MIN_PULSE_US   = 1000;   // 1ms 脉宽对应 0°
const int MAX_PULSE_US   = 2000;   // 2ms 脉宽对应 180°

/**
 * @brief 输出指定脉宽的PWM信号，持续一段时间（单位：毫秒）
 *
 * @param line 控制的GPIO线路
 * @param pulse_width_us 脉宽，单位：微秒
 * @param period_us PWM周期，单位：微秒
 * @param duration_ms 输出持续时间，单位：毫秒
 */
void generate_pwm(gpiod_line* line, int pulse_width_us, int period_us, int duration_ms) {
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time).count() < duration_ms) {
        // 输出高电平保持脉宽时间
        if (gpiod_line_set_value(line, 1) < 0) {
            std::cerr << "设置高电平失败" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_width_us));
        // 输出低电平保持周期剩余时间
        if (gpiod_line_set_value(line, 0) < 0) {
            std::cerr << "设置低电平失败" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(period_us - pulse_width_us));
    }
}

/**
 * @brief 平滑旋转舵机：从 0°（1ms脉宽）增加到180°（2ms脉宽），
 *        旋转结束后在180°位置保持一段时间，然后退出。
 */
void rotateServo180() {
    // 打开GPIO芯片
    gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "无法打开GPIO芯片 " << GPIO_CHIP << std::endl;
        return;
    }
    // 获取对应GPIO线路
    gpiod_line* line = gpiod_chip_get_line(chip, SERVO_PIN);
    if (!line) {
        std::cerr << "无法获取GPIO线路 " << SERVO_PIN << std::endl;
        gpiod_chip_close(chip);
        return;
    }
    // 请求该GPIO线路为输出模式，初始状态为低电平
    if (gpiod_line_request_output(line, "rotate180", 0) < 0) {
        std::cerr << "无法将GPIO线路设置为输出模式" << std::endl;
        gpiod_chip_close(chip);
        return;
    }
   
    std::cout << "[rotateServo180] 开始平滑旋转从 0° 到 180°..." << std::endl;
   
    const int step_duration_ms = 50; // 每步输出50ms
    const int steps = 20;            // 分成20步平滑增加
    int step_size = (MAX_PULSE_US - MIN_PULSE_US) / steps;
    int pulse_width = MIN_PULSE_US;
   
    // 逐步增加脉宽，从0°平滑旋转到180°
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width += step_size;
    }
   
    // 在180°位置保持一段时间（例如1秒）
    std::cout << "[rotateServo180] 舵机已旋转到180°，保持当前位置。" << std::endl;
    generate_pwm(line, MAX_PULSE_US, PWM_PERIOD_US, 1000);
   
    // 停止PWM输出并清理资源
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);
   
    std::cout << "[rotateServo180] 舵机旋转操作完成。" << std::endl;
}
