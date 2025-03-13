# 🎙️ Whisper Speech Recognition Project

This project is based on OpenAI's Whisper speech recognition system and uses [`whisper.cpp`](https://github.com/ggerganov/whisper.cpp) for **local** speech-to-text processing.  

**⚠️ Note:** This repository **does not include `whisper.cpp`**. Please follow the steps below to download and install it.

---

## 🚀 1. Install `whisper.cpp`

### 1️⃣ Clone `whisper.cpp`
Run the following command to download `whisper.cpp` into the `external/` directory:

```bash
git clone https://github.com/ggerganov/whisper.cpp.git external/whisper.cpp
```

Then navigate to the `whisper.cpp` directory:

```bash
cd external/whisper.cpp
```

---
### Configuration dependency

```bash
 -x ./build_environment.sh
```

### 2️⃣ Build `whisper.cpp`
Since Whisper requires compilation, run the following commands:

```bash
cmake -B build
cmake --build build --config Release
```

---

## 📌 **Test if Whisper is working correctly**  
If Whisper runs successfully, the installation is **complete** 🎉.

```bash
./build/bin/whisper-cli -m models/ggml-base.en-q5_1.bin ./samples/jfk.wav
```

---

## 🚀 2. Run This Project

### 1️⃣ Compile the project

```bash
mkdir build
cd build
cmake ..
make
```

---

### 2️⃣ Start the Whisper server

```bash
./start_whisper_server.sh
```

---

### 3️⃣ Run Speech Recognition
Execute the following command:

```bash
./whisper ../config.json
```

---

## 📌 **This process will:**
- 🎙️ Record audio  
- 🔄 Send it to the local `whisper.cpp` server  
- 📝 Retrieve the speech-to-text result  
- ✅ Display the recognized text  

---

## 📌 JSON Support

This project includes [`json.hpp`](https://github.com/nlohmann/json) from the **JSON for Modern C++** library.  
It provides a modern, easy-to-use JSON parser for C++.
.........
