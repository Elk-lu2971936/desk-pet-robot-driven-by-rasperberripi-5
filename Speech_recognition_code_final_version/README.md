# Whisper-Fuzzy DeskPet — Voice-Controlled Companion for Raspberry Pi

A real-time, offline speech recognition system using Whisper.cpp and fuzzy matching, designed for controlling four servo motors and an OLED screen on a Raspberry Pi-based desk pet.

## Project Context

This project was developed as part of a group coursework. It integrates speech recognition with hardware interaction and demonstrates how to use Whisper.cpp with custom command mapping for embedded systems.

## 🔄 Project Evolution Notice

This project was originally built on top of `whisper-server`, using an HTTP-based architecture to perform speech recognition.

While functional, that approach introduced unnecessary communication overhead and complexity.

🧠 **This new version** eliminates the HTTP server and instead **integrates Whisper C++ directly into the application** for better efficiency, simplicity, and real-time performance.

### Key Improvements Over the First Version:

- ⚡ No HTTP communication, Whisper runs fully embedded
- 🎙️ Real-time speech recognition with low latency
- 🛠️ GPIO servo control triggered instantly after voice match
- ✅ Easier to deploy and maintain

If you're looking for the original HTTP-based architecture, check out the earlier branch or release tagged `Speech Recognition Code (Version 1)`.

## Overview

- Converts spoken input into matched command codes via a custom `config.json` file
- Triggers hardware actions via callback (e.g., servo movement, OLED updates)
- Supports real-time recognition with VAD modes
- Modular and lightweight, fully compatible with Raspberry Pi

## Features

- Whisper.cpp integration for local speech recognition
- Custom text-to-code mapping using JSON configuration
- Audio input through SDL2
- Hardware interaction via callback dispatch

## Build & Run

```bash
cmake -B build -DWHISPER_SDL2=ON
cmake --build build
```

## Example config.json

```json
[
  { "text": ["stand up."], "code": "0x06" },
  { "text": ["okay"], "code": "0x03" }
]
```

## License

MIT License for original code. Third-party libraries retain their licenses.

# 🧪 How to Run

## 📦 Prepare Environment

### 1. Install dependencies

```bash
bash -x ./build_environment.sh
```

### 2. Compile whisper.cpp

```bash
cmake -B build -DWHISPER_SDL2=ON -DENABLE_GDB=OFF -S whisper.cpp
cmake --build build --config Release
```

### 3. Verify installation

```bash
./build/bin/whisper-cli -m ./whisper.cpp/models/ggml-base.en-q5_1.bin ./whisper.cpp/samples/jfk.wav
cd ..
```

---

## ⚙️ Configuration

### config.json (command mapping)

```json
[
  {
    "text": ["stand up."],
    "code": "0x06"
  },
  {
    "text": ["okay"],
    "code": "0x03"
  },
  {
    "text": ["sleep."],
    "code": "0x05"
  }
]
```

---

## 📥 Download Model

```bash
cd whisper.cpp/models/
./download-ggml-model.sh base.en-q5_1
```

---

## 🚀 Run Modes

### 🎙️ VAD Mode (Only respond when you speak)

```bash
./build/bin/whisper-fuzzy -u ./config.json -m ./whisper.cpp/models/ggml-base.en-q5_1.bin -t 6 --step 0 --length 3000 -vth 0.6
```

---
