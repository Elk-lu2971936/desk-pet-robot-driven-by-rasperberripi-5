#pragma once

/**
 * @brief 同时使两个伺服电机从 0° 平滑旋转至 180°并保持位置。
 *
 * 保证两个伺服电机（分别接在 GPIO13 和 GPIO12）均以相同的幅度旋转。
 */
void rotateServo180();

/**
 * @brief 同时使两个伺服电机从 180° 平滑旋转回 0°并保持位置，用于 sleep 模式。
 *
 * 保证两个伺服电机（分别接在 GPIO13 和 GPIO12）均以相同的幅度回到初始位置（0°）。
 */
void rotateServo0();

/**
 * @brief 同时使两个舵机实现 move forward 操作：
 *        GPIO13 的舵机平滑从 0° 旋转到 90°；
 *        GPIO12 的舵机平滑从 180° 旋转到 90°。
 */
void moveForward();

/**
 * @brief 交替执行舵机运动：
 *        先使 GPIO12 舵机从 180° 平滑旋转到 90°，
 *        再使 GPIO13 舵机从 0° 平滑旋转到 180°，
 *        共交替运行 6 个周期。
 */
void alternateRotation();
