#include "whisper_fuzzy.h"
#include "ServoController.h"
#include <iostream>
#include <thread>
#include <cstring>
#include <map>
#include <functional>

using ActionCallback = std::function<void()>;
std::map<std::string, ActionCallback> commandMap;

static int whisper_user_callback(size_t, const char*, const char* code, void* userdata) {
    if (!code) return -1;
    std::string cmd(code);
    if (commandMap.count(cmd)) commandMap[cmd]();
    return 0;
}

int main(int argc, char* argv[]) {
    whisper_fuzzy_t* w = whisper_fuzzy_init(argc, argv);
    if (!w) return -1;

    ServoController controller;
    commandMap["0x06"] = [&]() { controller.standUp(); };
    commandMap["0x05"] = [&]() { controller.sleep(); };
    commandMap["0x00"] = [&]() { controller.alternate(); };

    whisper_fuzzy(w, whisper_user_callback, nullptr);
    whisper_fuzzy_exit(w);
    return 0;
}
