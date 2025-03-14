#include <iostream>
#include <chrono>
#include <thread>
#include <gpiod.h>

#define GPIO_CHIP   "/dev/gpiochip0"
#define SERVO_PIN   13  // Assumed using GPIO number 13

// PWM parameter settings
const int PWM_PERIOD_US = 20000;       // PWM period: 20ms (50Hz)
const int NEUTRAL_PULSE_US = 1500;       // Neutral position pulse width: 1.5ms

// Generates a PWM signal with a specified pulse width for a given duration (in milliseconds) using software PWM
void generate_pwm(struct gpiod_line* line, int pulse_width_us, int period_us, int duration_ms) {
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start_time).count() < duration_ms) {
        // Set output high for the pulse width duration
        if (gpiod_line_set_value(line, 1) < 0) {
            std::cerr << "Failed to set high level" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(pulse_width_us));
        
        // Set output low for the remainder of the period
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
    
    // Get the corresponding GPIO line (SERVO_PIN)
    struct gpiod_line* line = gpiod_chip_get_line(chip, SERVO_PIN);
    if (!line) {
        std::cerr << "Unable to get GPIO line " << SERVO_PIN << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }
    
    // Request the GPIO line as an output, initially set to low
    if (gpiod_line_request_output(line, "servo_control", 0) < 0) {
        std::cerr << "Unable to set GPIO line as output" << std::endl;
        gpiod_chip_close(chip);
        return 1;
    }
    
    std::cout << "SG90 Servo Control: Starting to output PWM signal with neutral pulse width (1.5ms)" << std::endl;
    
    // Continuously output the PWM signal for 5 seconds (servo remains in neutral position during this period)
    generate_pwm(line, NEUTRAL_PULSE_US, PWM_PERIOD_US, 5000);
    
    // Stop the PWM output, ensuring the output is low
    gpiod_line_set_value(line, 0);
    
    // Release resources
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    
    std::cout << "PWM output ended, resources have been released." << std::endl;
    return 0;
}

