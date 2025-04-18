#include "oled_display.h"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <cstring>
#include <cstdint>
#include <chrono>
#include <thread>
#include <cmath>

#define OLED_ADDR     0x3C
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define PAGE_COUNT    (SCREEN_HEIGHT / 8)

// 内部函数：打开 I2C 设备并设置 OLED 地址
static int openOLED() {
    const char* i2c_device = "/dev/i2c-1";
    int file = open(i2c_device, O_RDWR);
    if (file < 0) {
        std::cerr << "Unable to open I2C device: " << i2c_device << std::endl;
        return -1;
    }
    if (ioctl(file, I2C_SLAVE, OLED_ADDR) < 0) {
        std::cerr << "Unable to connect to OLED at address 0x3C" << std::endl;
        close(file);
        return -1;
    }
    return file;
}

// 内部函数：发送单个命令到OLED
static void sendCommand(int file, uint8_t cmd) {
    uint8_t buffer[2] = {0x00, cmd};
    write(file, buffer, 2);
}

// 内部函数：发送一块数据到OLED（例如某页的数据）
static void sendData(int file, const uint8_t *data, size_t length) {
    uint8_t* buffer = new uint8_t[length + 1];
    buffer[0] = 0x40; // 控制字节，表示后续均为数据
    memcpy(&buffer[1], data, length);
    write(file, buffer, length + 1);
    delete[] buffer;
}

// 内部函数：发送OLED初始化命令序列（适用于 SSD1306）
static void initOLEDHardware(int file) {
    sendCommand(file, 0xAE); // Display off
    sendCommand(file, 0xD5);
    sendCommand(file, 0x80);
    sendCommand(file, 0xA8);
    sendCommand(file, 0x3F);
    sendCommand(file, 0xD3);
    sendCommand(file, 0x00);
    sendCommand(file, 0x40);
    sendCommand(file, 0x8D);
    sendCommand(file, 0x14);
    sendCommand(file, 0x20);
    sendCommand(file, 0x00);
    sendCommand(file, 0xA1);
    sendCommand(file, 0xC8);
    sendCommand(file, 0xDA);
    sendCommand(file, 0x12);
    sendCommand(file, 0x81);
    sendCommand(file, 0xCF);
    sendCommand(file, 0xD9);
    sendCommand(file, 0xF1);
    sendCommand(file, 0xDB);
    sendCommand(file, 0x40);
    sendCommand(file, 0xA4);
    sendCommand(file, 0xA6);
    sendCommand(file, 0xAF); // Display on
}

// 内部函数：将数据缓冲区写入OLED，每页写入一次
static void drawBuffer(int file, const uint8_t *buffer) {
    for (uint8_t page = 0; page < PAGE_COUNT; ++page) {
        sendCommand(file, 0xB0 + page);
        sendCommand(file, 0x00);
        sendCommand(file, 0x10);
        sendData(file, &buffer[page * SCREEN_WIDTH], SCREEN_WIDTH);
    }
}

/*
   生成“stand up”表情（⊙▽⊙）的图案数据：
   - 首先将整个缓冲区置为 0xFF（全白背景）；
   - 然后在指定区域“清除”像素（置0），形成两只眼和一个倒三角形嘴，
     从而在白色背景上呈现出黑色颜文字。
*/
static void generateStandUp(uint8_t *buffer) {
    memset(buffer, 0xFF, PAGE_COUNT * SCREEN_WIDTH); // 全白背景
    int center_x = SCREEN_WIDTH / 2;    // 64
    int center_y = SCREEN_HEIGHT / 2;     // 32
    // 绘制两只眼——以圆形绘制
    int eye_radius = 5;
    int left_eye_cx = center_x - 20;
    int left_eye_cy = center_y - 10;
    int right_eye_cx = center_x + 20;
    int right_eye_cy = center_y - 10;
    // 遍历所有像素，根据判断条件决定是否“清除”该像素（置0表示黑）
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            bool drawPixel = false;
            // 左眼圆区域
            int dx = x - left_eye_cx;
            int dy = y - left_eye_cy;
            if (dx * dx + dy * dy <= eye_radius * eye_radius)
                drawPixel = true;
            // 右眼圆区域
            dx = x - right_eye_cx;
            dy = y - right_eye_cy;
            if (dx * dx + dy * dy <= eye_radius * eye_radius)
                drawPixel = true;
            // 绘制嘴：利用简单的倒V形（▽），在脸中央下部绘制
            if (y >= center_y + 5 && y <= center_y + 15) {
                int offset = y - (center_y + 5);
                int left_bound = center_x - offset - 5;
                int right_bound = center_x + offset + 5;
                if (x >= left_bound && x <= right_bound) {
                    // 为了形成倒V形，排除中间部分（保留白色）稍微突出两侧
                    if (std::abs(x - center_x) > offset)
                        drawPixel = true;
                }
            }
            if (drawPixel) {
                int page = y / 8;
                int index = page * SCREEN_WIDTH + x;
                buffer[index] &= ~(1 << (y % 8)); // 清除对应位（设置为0，显示为黑）
            }
        }
    }
}

/*
   生成“sleep”表情 ((￣_,￣ )) 的图案数据：
   - 先将整个缓冲区填充为 0xFF（全白背景）；
   - 然后以“短横线”来绘制两只半闭的眼，
     以及在嘴部区域绘制一条直线，形成简单的睡眼和疲惫表情。
*/
static void generateSleep(uint8_t *buffer) {
    memset(buffer, 0xFF, PAGE_COUNT * SCREEN_WIDTH);
    int center_x = SCREEN_WIDTH / 2;    // 64
    int center_y = SCREEN_HEIGHT / 2;     // 32
    // 眼部：以短横线表示
    int left_eye_x = center_x - 20;
    int right_eye_x = center_x + 20;
    int eye_y = center_y - 10;
    for (int y = eye_y - 1; y <= eye_y + 1; y++) {
        for (int x = left_eye_x - 3; x <= left_eye_x + 3; x++) {
            int page = y / 8;
            int index = page * SCREEN_WIDTH + x;
            buffer[index] &= ~(1 << (y % 8));
        }
        for (int x = right_eye_x - 3; x <= right_eye_x + 3; x++) {
            int page = y / 8;
            int index = page * SCREEN_WIDTH + x;
            buffer[index] &= ~(1 << (y % 8));
        }
    }
    // 嘴部：绘制一条直线
    for (int y = center_y + 10; y <= center_y + 11; y++) {
        for (int x = center_x - 10; x <= center_x + 10; x++) {
            int page = y / 8;
            int index = page * SCREEN_WIDTH + x;
            buffer[index] &= ~(1 << (y % 8));
        }
    }
}

void initOLED() {
    int file = openOLED();
    if (file < 0)
        return;
    initOLEDHardware(file);
    close(file);
}

void showStandUp() {
    int file = openOLED();
    if (file < 0)
        return;
    uint8_t* display_buffer = new uint8_t[PAGE_COUNT * SCREEN_WIDTH];
    generateStandUp(display_buffer);
    drawBuffer(file, display_buffer);
    delete[] display_buffer;
    close(file);
}

void showSleep() {
    int file = openOLED();
    if (file < 0)
        return;
    uint8_t* display_buffer = new uint8_t[PAGE_COUNT * SCREEN_WIDTH];
    generateSleep(display_buffer);
    drawBuffer(file, display_buffer);
    delete[] display_buffer;
    close(file);
}
