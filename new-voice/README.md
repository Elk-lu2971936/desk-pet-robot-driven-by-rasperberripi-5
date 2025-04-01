# Whisper-Fuzzy DeskPet — Voice-Controlled Companion for Raspberry Pi

A lightweight, offline-capable voice recognition system using Whisper.cpp and fuzzy matching to control four servos and an OLED screen on a desk pet. Built in C++ for Raspberry Pi.

## Project Overview

This project is a real-time speech recognition system that listens to voice commands, converts them into matching control codes using a `config.json` file, and then triggers actions such as moving servos or updating the OLED screen on a desk pet.

It supports two recognition modes:
- Sliding window mode (periodic recognition)
- Voice Activity Detection (VAD) mode (triggered only when someone speaks)

## Features

- Real-time, offline voice recognition using Whisper.cpp
- Customizable text to command code mapping via `config.json`
- Callback-based design for flexible hardware integration
- Lightweight and suitable for Raspberry Pi
- SDL2-based audio input and VAD support
- Modular CMake-based build system

## Dependencies

Install required packages (tested on Raspberry Pi OS / Ubuntu):

```bash
sudo apt update
sudo apt install -y cmake build-essential libsdl2-dev libasound2-dev libfftw3-dev
```

## Build and Run

### Step 1: Download Whisper Model

```bash
mkdir -p models
cd models
wget https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en-q5_1.bin
cd ..
```

You can replace the model file with others (tiny, small, medium) depending on speed versus accuracy tradeoff.

### Step 2: Build the Project

```bash
cmake -B build -DWHISPER_SDL2=ON
cmake --build build
```

### Step 3: Run the Program

#### Mode 1: VAD (Voice Activated)

```bash
./build/bin/whisper-fuzzy   -u config.json   -m models/ggml-base.en-q5_1.bin   --step 0 --length 5000 -vth 0.6
```

#### Mode 2: Periodic Recognition (Sliding Window)

```bash
./build/bin/whisper-fuzzy   -u config.json   -m models/ggml-base.en-q5_1.bin   --step 1000 --length 3000
```

## Configuration File: `config.json`

```json
[
  { "text": ["hello", "hi"], "code": "0x01" },
  { "text": ["okay", "okay."], "code": "0x03" },
  { "text": ["turn left"], "code": "0x10" }
]
```

## Project Structure

```
.
├── main.cpp
├── whisper_fuzzy.cpp/h
├── whisper_stream.cpp/h
├── debug.cpp/h
├── config.json
├── json.hpp
├── build_environment.sh
├── models/
├── CMakeLists.txt
└── README.md
```

## .gitignore Suggestions

```gitignore
build/
models/*.bin
*.wav
*.log
*.o
*.so
.vscode/
.idea/
```

## Contributors

| Member | Role |
|--------|------|
| You | Voice recognition and command matching system |
| Teammate A | Servo motor control integration |
| Teammate B | OLED display updates |
| Teammate C | Desk pet behavior design and testing |

## License

MIT License

## Collaboration

If you'd like to use or contribute to this project, feel free to fork, submit issues, or open pull requests.