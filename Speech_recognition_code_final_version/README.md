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

If you're looking for the original HTTP-based architecture, check out the earlier branch or release tagged `v0.9-http`.

## Overview

- Converts spoken input into matched command codes via a custom `config.json` file
- Triggers hardware actions via callback (e.g., servo movement, OLED updates)
- Supports real-time recognition with VAD or periodic recognition modes
- Modular and lightweight, fully compatible with Raspberry Pi

## Features

- Whisper.cpp integration for local speech recognition
- Custom text-to-code mapping using JSON configuration
- Audio input through SDL2
- VAD-based and timer-based recognition support
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
