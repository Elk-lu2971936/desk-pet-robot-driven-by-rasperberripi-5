#pragma once

/**
 * @brief Smoothly rotate the servo from 0° to 180° and hold at 180°.
 *
 * 该函数使用软件 PWM 平滑增加脉宽，将舵机从 0° 旋转到 180°，
 * 然后在180°位置保持一段时间，最终停住，不再回转。
 */
void rotateServo180();
