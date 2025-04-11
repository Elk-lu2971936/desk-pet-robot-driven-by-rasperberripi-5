#pragma once

/**
 * @brief 同时使两个伺服电机从 0° 平滑旋转至 180°并保持位置。
 *
 * 采用相同的 PWM 脉宽控制，即统一从 MIN_PULSE_US（对应 0°）平滑增加到 MAX_PULSE_US（对应 180°），  
 * 保证两个伺服电机（分别接在 GPIO13 和 GPIO12）均以相同的幅度旋转。
 */
void rotateServo180();
