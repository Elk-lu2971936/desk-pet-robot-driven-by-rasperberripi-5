#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

/**
 * @brief 初始化SSD1306 OLED显示屏
 */
void initOLED();

/**
 * @brief 在OLED上显示 stand up 表情（⊙▽⊙），背景全白，颜文字以黑色呈现
 */
void showStandUp();

/**
 * @brief 在OLED上显示 sleep 表情 ((￣_,￣ )), 背景全白，颜文字以黑色呈现
 */
void showSleep();

#endif // OLED_DISPLAY_H
