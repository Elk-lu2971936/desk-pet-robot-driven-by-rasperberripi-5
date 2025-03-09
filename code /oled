#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <cstdint>
#include <gpiod.h>

#define I2C_DEV "/dev/i2c-1"   // Raspberry Pi I2C device
#define OLED_ADDR 0x3C         // SSD1315 I2C address (could be 0x3C or 0x3D)
#define GPIOCHIP "/dev/gpiochip0"
#define OLED_RESET_PIN 4       // Reset pin (assumed connected to GPIO4)

// Send a command to the OLED (via I2C)
void oled_command(int file, uint8_t command) {
    uint8_t buffer[2] = {0x00, command};  // 0x00 indicates command mode
    if (write(file, buffer, 2) != 2) {
        std::cerr << "I2C command transmission failed: 0x" << std::hex << int(command) << std::endl;
    }
}

// Reset the OLED display (using libgpiod for GPIO control)
void oled_reset() {
    struct gpiod_chip *chip = gpiod_chip_open(GPIOCHIP);
    if (!chip) {
        std::cerr << "Failed to open GPIO chip" << std::endl;
        return;
    }

    struct gpiod_line *line = gpiod_chip_get_line(chip, OLED_RESET_PIN);
    if (!line) {
        std::cerr << "Failed to get GPIO line" << std::endl;
        gpiod_chip_close(chip);
        return;
    }

    if (gpiod_line_request_output(line, "oled_reset", 0) < 0) {
        std::cerr << "Failed to request GPIO output" << std::endl;
        gpiod_chip_close(chip);
        return;
    }

    gpiod_line_set_value(line, 0);  // Pull reset low
    usleep(100000);  // Wait for 100ms
    gpiod_line_set_value(line, 1);  // Release reset
    usleep(100000);  // Wait for OLED startup

    gpiod_line_release(line);
    gpiod_chip_close(chip);
}

// Initialize the SSD1315 OLED
void oled_init(int file) {
    oled_command(file, 0xAE);  // Turn off display
    oled_command(file, 0xD5);  // Set clock divider ratio
    oled_command(file, 0x80);  // Recommended value
    oled_command(file, 0xA8);  // Set multiplex ratio
    oled_command(file, 0x3F);  // 64 lines (128x64 screen)
    oled_command(file, 0xD3);  // Set display offset
    oled_command(file, 0x00);  // No offset
    oled_command(file, 0x40);  // Set start line to 0
    oled_command(file, 0xA1);  // Mirror horizontally (0xA0: normal, 0xA1: mirrored)
    oled_command(file, 0xC8);  // Mirror vertically (0xC0: normal, 0xC8: mirrored)
    oled_command(file, 0xDA);  // Set COM pin hardware configuration
    oled_command(file, 0x12);
    oled_command(file, 0x81);  // Set contrast control
    oled_command(file, 0x7F);  // Recommended value
    oled_command(file, 0xA4);  // Disable entire display on (A4: normal, A5: all on)
    oled_command(file, 0xA6);  // Set normal display (A6: normal, A7: inverse)
    oled_command(file, 0xD9);  // Set pre-charge period
    oled_command(file, 0xF1);  // SSD1315 recommended value
    oled_command(file, 0xDB);  // Set VCOMH deselect level
    oled_command(file, 0x40);
    oled_command(file, 0x8D);  // Enable charge pump
    oled_command(file, 0x14);
    oled_command(file, 0xAF);  // Turn on display
}

// Run initialization
int main() {
    oled_reset();  // Reset the OLED

    // Open the I2C device
    int file = open(I2C_DEV, O_RDWR);
    if (file < 0) {
        std::cerr << "Failed to open I2C device" << std::endl;
        return 1;
    }

    // Set the OLED I2C address
    if (ioctl(file, I2C_SLAVE, OLED_ADDR) < 0) {
        std::cerr << "Failed to communicate with OLED device" << std::endl;
        close(file);
        return 1;
    }

    oled_init(file);  // Initialize the OLED

    std::cout << "OLED initialization completed" << std::endl;

    close(file);
    return 0;
}
