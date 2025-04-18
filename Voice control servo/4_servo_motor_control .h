#pragma once

/**
 * @brief Simultaneously rotate both servos smoothly from 0° to 180° and hold the position.
 *
 * Ensures that both servos (connected to GPIO13 and GPIO12) rotate with the same angle and speed.
 */
void rotateServo180();

/**
 * @brief Simultaneously rotate both servos smoothly from 180° back to 0° and hold the position. Used for sleep mode.
 *
 * Ensures that both servos (connected to GPIO13 and GPIO12) return to the initial position (0°) with the same angle.
 */
void rotateServo0();

/**
 * @brief Perform a move forward action using two servos simultaneously:
 *        The servo on GPIO13 rotates smoothly from 0° to 90°;
 *        The servo on GPIO12 rotates smoothly from 180° to 90°.
 */
void moveForward();

/**
 * @brief Alternating servo motion:
 *        First, the servo on GPIO12 rotates smoothly from 180° to 90°;
 *        Then, the servo on GPIO13 rotates smoothly from 0° to 180°;
 *        This cycle repeats 6 times.
 */
void alternateRotation();
