# desk-pet-robot-driven-by-raspberrypi-5

A Raspberry Pi-based desktop pet robot with real-time motion control.

# Desktop Pet Robot

This project is a Raspberry Pi 5-based desktop pet robot that is driven by voice or manual commands (e.g., sit, stand, move forward/backward) in real-time.

# Features

- Voice or manual command control  
- Real-time response to commands (sit, stand, move forward/backward)  
- Smooth motion control using Raspberry Pi 5  

## Realtime Processing

The robot is designed to meet real-time requirements:

- **Low Latency**: Commands are processed within milliseconds to ensure immediate response.  
- **Precision Control**: The Raspberry Pi 5's multi-core CPU and real-time operating system (RTOS) ensure smooth and accurate motion control.  
- **Real-Time Feedback**: Sensors and actuators are continuously monitored to adjust movements dynamically.  

# Code Overview

## Speech Recognition Code

The speech recognition module in this project is based on the Whisper model. It uses SDL2 to capture live audio input from the microphone, performs real-time transcription of voice commands (e.g., "stand up", "sit down"), and maps them to predefined control codes (e.g., 0x06). These codes are then passed to the actuator control system through a callback function, enabling responsive and natural interaction.

Key features of the module include:

- Real-time audio input processing (supports VAD mode)  
- Voice transcription and fuzzy matching  
- Command-to-code mapping logic  
- Callback-based integration with the motion control module  

The full implementation of the speech recognition module is located in:

`Speech_recognition_code_final_version/`
