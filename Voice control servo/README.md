# 🐾 Desk Pet Servo Control System

This project implements a **voice-controlled servo motor system** for a Raspberry Pi-powered desk pet robot. It uses **C++ object-oriented programming**, **callback interfaces**, and a cute **OLED emoji display** for real-time interaction and feedback.

---

## 📦 Project Structure

```
desk-pet-robot/
├── main.cpp                  # Entry point: integrates whisper + servo
├── Servo.h / Servo.cpp       # Class to control a single GPIO-based servo
├── ServoController.h / .cpp  # Controls dual servo actions + OLED feedback
├── oled_display.h / .cpp     # Emoji rendering via SSD1306 OLED
├── whisper_fuzzy.h / .cpp    # Voice recognition logic
├── CMakeLists.txt            # Build config
```

---

## ✨ Features

- **Object-Oriented Servo Control**
  - `Servo` class wraps GPIO & PWM logic
  - `ServoController` coordinates two servos (GPIO13 & GPIO12)

- **Voice Command Callback Mechanism**
  - Uses `std::function + std::map`
  - Modular and easily extendable

- **OLED Emoji Display**
  - Displays standing 😃 and sleeping 😴 expressions
  - Linked to servo status changes

---

## 🎙️ Voice Command Mapping

| Code   | Action           | Servo Behavior          | OLED Emoji |
|--------|------------------|--------------------------|-------------|
| 0x06   | Stand up         | Rotate both to 180°      | `⊙▽⊙`       |
| 0x05   | Sleep mode       | Rotate both to 0°        | `((￣_,￣))` |
| 0x00   | Alternate crawl  | 6x cycle forward/back    | *(None)*    |

---

## 🔁 Callback-Based Architecture

```cpp
// Mapping voice command to servo action
commandMap["0x06"] = [&]() { controller.standUp(); };
commandMap["0x05"] = [&]() { controller.sleep(); };
commandMap["0x00"] = [&]() { controller.alternate(); };

// Callback invoked by whisper recognizer
static int whisper_user_callback(..., const char* code, ...) {
    if (commandMap.count(code)) commandMap[code]();
    return 0;
}
```

---

## 🧱 Class Diagram (Text View)

```
+----------------------+           +--------------------------+
|      main.cpp        |  --->     | whisper_user_callback()  |
+----------------------+           +--------------------------+
           |                                   |
           v                                   v
 +------------------------+     +------------------------------+
 |   ServoController      | <-> | std::map<std::string, func> |
 +------------------------+     +------------------------------+
 | - Servo left           |
 | - Servo right          |
 | - displayStatus()      |
 +------------------------+
      |            |
      v            v
  +--------+   +--------+
  | Servo  |   | Servo  |
  +--------+   +--------+
   (GPIO13)     (GPIO12)
```

---

## 🧪 How to Build & Run

```bash
mkdir build && cd build
cmake ..
make -j4
./desk_pet_servo
```

---


