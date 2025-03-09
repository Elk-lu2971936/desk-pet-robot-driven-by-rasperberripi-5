#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>

#define GPIO_CHIP   "/dev/gpiochip0"
#define SERVO_PIN   13  // Using GPIO number 13

// PWM parameter settings
const int PWM_PERIOD_US = 20000;  // 20ms period (50Hz)
const int MIN_PULSE_US   = 1000;   // 1ms pulse width corresponds to 0°
const int MAX_PULSE_US   = 2000;   // 2ms pulse width corresponds to 180°

/**
 * @brief Outputs a specified pulse width using software PWM for a given duration (in milliseconds).
 *
 * @param line The GPIO line to control.
 * @param pulse_width_us Pulse width in microseconds.
 * @param period_us PWM period in microseconds.
 * @param duration_ms Duration of the output in milliseconds.
 */
void generate_pwm(struct gpiod_line* line, int pulse_width_us, int period_us, int duration_ms) {
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time).count() < duration_ms) {
        // Output high level for the duration of the pulse width
        if (gpiod_line_set_value(line, 1) < 0) {
            std::cerr << "Failed to set high level" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_width_us));
        // Output low level for the remaining period time
        if (gpiod_line_set_value(line, 0) < 0) {
            std::cerr << "Failed to set low level" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(period_us - pulse_width_us));
    }
}

int main() {
    // Open the GPIO chip
    struct gpiod_chip* chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        std::cerr << "Unable to open GPIO chip " << GPIO_CHIP << std::endl;
        return 1;
    }

    // Get the corresponding GPIO line
    struct gpiod_line* line = gpiod_chip_get_line(chip, SERVO_PIN);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << SERVO_PIN << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }

    // Request the GPIO line as output mode, initial state is low
    if (gpiod_line_request_output(line, "servo_control", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }

    std::cout << "Starting servo 180-degree rotation..." << std::endl;

    // To achieve a 180° rotation (from 0° to 180°), gradually increase the pulse width
    // from MIN_PULSE_US to MAX_PULSE_US.
    // The motion is divided into several small steps, each lasting a certain duration.
    const int step_duration_ms = 50; // Each step outputs for 50ms
    const int steps = 20;            // Divided into 20 steps
    int step_size = (MAX_PULSE_US - MIN_PULSE_US) / steps;  // Pulse width increment per step

    int pulse_width = MIN_PULSE_US;
    // Smoothly increase from 0° (1ms pulse width) to 180° (2ms pulse width)
    for (int i = 0; i <= steps; i++) {
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
        pulse_width += step_size;
    }

    // Optional: if you want the servo to return to the initial position, smoothly decrease the pulse width
    for (int i = 0; i <= steps; i++) {
        pulse_width -= step_size;
        generate_pwm(line, pulse_width, PWM_PERIOD_US, step_duration_ms);
    }

    // Stop the PWM signal, ensuring the output remains low
    gpiod_line_set_value(line, 0);
    gpiod_line_release(line);
    gpiod_chip_close(chip);

    std::cout << "Servo 180-degree rotation operation completed." << std::endl;
    return 0;
}
