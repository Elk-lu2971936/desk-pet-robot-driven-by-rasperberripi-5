#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cmath>

#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define PAGE_COUNT (SCREEN_HEIGHT / 8)

/**
 * Sends a single command to the OLED.
 */
void sendCommand(int file, uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd};
    write(file, buffer, 2);
}

/**
 * Sends a block of data to the OLED (for example, data for an entire page).
 */
void sendData(int file, const uint8_t *data, size_t length) {
    uint8_t buffer[length + 1];
    buffer[0] = 0x40;  // Control byte indicating that the following bytes are display data
    memcpy(&buffer[1], data, length);
    write(file, buffer, length + 1);
}

/**
 * Dynamically generates the smiley face pattern data (128×64) and stores it in buffer.
 * Display concept:
 *  - Use a large circle to represent the face, with the center at the screen's center and a radius of 28.
 *  - Draw two eyes inside the face (left eye: centered at (center_x-12, center_y-8); right eye: centered at (center_x+12, center_y-8), with a radius of 3).
 *  - Draw the mouth in the lower part of the face by turning off pixels along two horizontal lines
 *    (from x = center_x-10 to x = center_x+10 at y positions center_y+10 and center_y+11).
 */
void generateSmiley(uint8_t *buffer) {
    // Clear the entire buffer
    memset(buffer, 0, PAGE_COUNT * SCREEN_WIDTH);
   
    int center_x = SCREEN_WIDTH / 2;    // 64
    int center_y = SCREEN_HEIGHT / 2;     // 32
    int face_radius = 28;                 // Radius of the face
    int eye_radius = 3;                   // Radius of the eyes
    int left_eye_cx = center_x - 12;
    int left_eye_cy = center_y - 8;
    int right_eye_cx = center_x + 12;
    int right_eye_cy = center_y - 8;
   
    // Iterate over every pixel (x, y)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            bool pixel = false;
            // If the pixel is inside the face circle, set it to on.
            int dx = x - center_x;
            int dy = y - center_y;
            if (dx * dx + dy * dy <= face_radius * face_radius) {
                pixel = true;
            }
            // In the eye regions, clear the pixel (set to off)
            int dx_eye = x - left_eye_cx;
            int dy_eye = y - left_eye_cy;
            if (dx_eye * dx_eye + dy_eye * dy_eye <= eye_radius * eye_radius) {
                pixel = false;
            }
            dx_eye = x - right_eye_cx;
            dy_eye = y - right_eye_cy;
            if (dx_eye * dx_eye + dy_eye * dy_eye <= eye_radius * eye_radius) {
                pixel = false;
            }
            // Draw the mouth: in the lower part of the face, draw a slightly thick horizontal line
            if ((y == center_y + 10 || y == center_y + 11) && (x >= center_x - 10 && x <= center_x + 10)) {
                pixel = false;
            }
            // Write the pixel to the buffer.
            // Note: The OLED stores data in pages: each page consists of 8 rows,
            // and each byte represents the 8 pixels in a column for that page.
            if (pixel) {
                int page = y / 8;                      // Determine the page
                int index = page * SCREEN_WIDTH + x;     // Index in the buffer
                buffer[index] |= (1 << (y % 8));         // Set the corresponding bit
            }
        }
    }
}

/**
 * Writes the generated pattern data to the OLED display.
 */
void drawSmiley(int file, const uint8_t *buffer) {
    for (uint8_t page = 0; page < PAGE_COUNT; ++page) {
        sendCommand(file, 0xB0 + page);  // Set page address (0xB0 ~ 0xB7)
        sendCommand(file, 0x00);         // Set lower column address
        sendCommand(file, 0x10);         // Set higher column address
        sendData(file, &buffer[page * SCREEN_WIDTH], SCREEN_WIDTH);
    }
}

int main() {
    const char *i2c_device = "/dev/i2c-1";
    int file = open(i2c_device, O_RDWR);
    if (file < 0) {
        std::cerr << "Unable to open I2C device" << std::endl;
        return 1;
    }

    if (ioctl(file, I2C_SLAVE, OLED_ADDR) < 0) {
        std::cerr << "Unable to connect to OLED" << std::endl;
        close(file);
        return 1;
    }
   
    // Generate the smiley face pattern data
    uint8_t display_buffer[PAGE_COUNT * SCREEN_WIDTH];
    generateSmiley(display_buffer);
   
    // Write the pattern data to the OLED
    drawSmiley(file, display_buffer);

    close(file);
    return 0;
}

