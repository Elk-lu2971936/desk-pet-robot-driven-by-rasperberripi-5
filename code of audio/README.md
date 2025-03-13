# 🎙️ Whisper Speech Recognition Project

This project is based on OpenAI's Whisper speech recognition system and uses [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp) for **local** speech-to-text processing.  

**⚠️ Note:** This repository **does not include `whisper.cpp`**. Please follow the steps below to download and install it.

---

## **🚀 1. Install `whisper.cpp`**
1️⃣ Clone `whisper.cpp
Run the following command to download `whisper.cpp` into the `external/` directory:
```bash
git clone https://github.com/ggerganov/whisper.cpp.git external/whisper.cpp
Then navigate to the whisper.cpp directory:
cd external/whisper.cpp
2️⃣ Build whisper.cpp
Since Whisper requires compilation, run the following commands:

cmake -B build
cmake --build build --config Release

📌Test if Whisper is working correctly
If Whisper runs successfully, the installation is complete 🎉.

## **🚀 2. Run This Project`**

1️⃣ Compile the project

mkdir build
cd build
cmake ..
make

2️⃣ Start the Whisper server

./start_whisper_server.sh

3️⃣ Run Speech Recognition
Execute the following command:

./whisper ../config.json

📌 This process will:
Record audio
Send it to the local whisper.cpp server
Retrieve the speech-to-text result
Display the recognized text
