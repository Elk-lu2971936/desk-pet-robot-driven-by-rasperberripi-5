# 🐾 Raspberry Pi 5 Powered Desk Pet Robot

A real-time desktop pet robot driven by Raspberry Pi 5 with speech-controlled smooth motion and feedback display.

---

## 📌 Project Overview

This project implements a **desk pet robot** powered by Raspberry Pi 5 that can respond in **real-time** to both **voice**  commands like:

- 🗣️ "Stand up"
- 💤 "Sleep"
- ↔️ "Crawl forward / backward"

---

## ✨ Features

- 🎙️ Voice or manual command control
- ⚡ Real-time reaction (Voice control is processed within 30ms, and servo/OLED control is completed within 60ms)
- 🤖 Smooth motion via multi-threaded servo control
- 🧠 Callback-based speech interpretation
- 🖥️ OLED emoji feedback display
- 🧩 Fully C++ object-oriented and modular design

---

## ⏱️ Real-Time Design Principles

- **Low Latency**: Whisper-based command recognition in < 1 sec
- **Precise Motion**: Dual servos (GPIO12/13) with PWM control
- **Live Feedback**: OLED shows real-time pet emotion
- **Thread Safety**: All servo actions are multithreaded
- **Callback Architecture**: Events are decoupled from motor logic

---

## 🧠 Speech Recognition Module

Based on the **Whisper** model, this module uses SDL2 to capture real-time mic input, transcribes commands, and maps to predefined motion codes (`e.g., 0x06` = stand up). These are then passed to the servo controller via callback.

📁 Located in:  
**`Speech_recognition_code_final_version/`**

### Key Features

- ✅ Real-time VAD-based audio input
- ✅ Whisper model integration
- ✅ Command-to-code mapping
- ✅ Callback execution to trigger motors

---

## 🔁 Motion Control via Callback & Encapsulation

The motion system is built using modern C++ principles:

📁 Located in:  
**`Voice control servo/final_servo_control/`**

- `Servo.h / Servo.cpp`: Single servo controller for GPIO
- `ServoController.h / .cpp`: Controls both servos and links OLED
- `oled_display.h / .cpp`: SSD1306 display logic
- `main.cpp`: Callback registration and whisper integration

```cpp
// Callback binding
commandMap["0x06"] = [&]() { controller.standUp(); };
commandMap["0x05"] = [&]() { controller.sleep(); };
commandMap["0x00"] = [&]() { controller.alternate(); };
```

---

## ✅ Summary of Course Requirements Met

| Requirement                             | Status   |
|----------------------------------------|----------|
| Real-time voice command processing     | ✅        |
| Multithreaded servo motor control      | ✅        |
| Callback-based event handling          | ✅        |
| Object-oriented C++ design             | ✅        |
| GitHub version control usage           | ✅        |
| No blocking `sleep()` delay logic      | ✅        |
| Raspberry Pi GPIO + OLED integration   | ✅        |

---

## 👥 Team 11 - Desk Pet Robot

- Qingyu Lu (2971936L)  
- Chaoqi Guo (2976693G)  
- Zeyu Zhao (2984894Z)  
- Yiyang Zhang (2990718Z)  
- Hao Zhong (2984282Z)
