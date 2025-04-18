#include "debug.h"
#include "whisper_fuzzy.h"
#include "rotate180.h"   // Include the header file for servo rotation interface

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>

/**
 * Whisper recognition callback function.
 * When matched:
 *   - code = "0x06": calls rotateServo180() to rotate the servo to 180° (standup);
 *   - code = "0x05": calls rotateServo0() to return the servo to 0° (sleep mode);
 *   - code = "0x00": performs alternating rotations:
 *         First, GPIO12 moves the servo from 180° to 90° ("forward" rotation),
 *         Then, GPIO13 moves the servo from 0° to 180° ("backward" rotation),
 *         Repeats this cycle 6 times.
 */
static int whisper_user_callback(size_t leat_count, const char *text, const char* code, void* userdata) {
    if (leat_count)
        return 1;
    if (!text || !code || !userdata) {
        LOG_ERR("args fail! text(%p), code(%p), userdata(%p)", text, code, userdata);
        return -1;
    }
    size_t &count = *(size_t *)userdata;
    ++count;
    LOG_INFO("[%zu] get text: %s, code: %s", count, text, code);

    if (strcmp(code, "0x06") == 0) {
        std::cout << "[Action] Matched code 0x06 => rotating both servos (GPIO13 & GPIO12) to 180°" << std::endl;
        rotateServo180();
    } else if (strcmp(code, "0x05") == 0) {
        std::cout << "[Action] Matched code 0x05 => rotating both servos (GPIO13 & GPIO12) to 0° (sleep mode)" << std::endl;
        rotateServo0();
    } else if (strcmp(code, "0x00") == 0) {
        std::cout << "[Action] Matched code 0x00 => performing alternating rotations:" << std::endl;
        alternateRotation();
    }
    return 0;
}

/**
 * Whisper task thread
 */
void whisper_fuzzy_task(void* userdata) {
    whisper_fuzzy_t* w = (whisper_fuzzy_t*)userdata;
    std::cout << "Task whisper is running on thread " << std::this_thread::get_id() << std::endl;
    size_t count = 0;
    int ret = whisper_fuzzy(w, whisper_user_callback, &count);
    LOG_INFO("whisper ret: %d", ret);
    std::cout << "Task whisper completed" << std::endl;
}

/**
 * Main function
 */
int main(int argc, char const* argv[]) {
    const int numThreads = 1;
    std::vector<std::thread> threads;
    whisper_fuzzy_t* w = whisper_fuzzy_init(argc, argv);
    if (!w) {
        LOG_ERR("fail to init whisper");
        return -1;
    }
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(whisper_fuzzy_task, w);
    }
    for (auto& th : threads) {
        th.join();
    }
    whisper_fuzzy_exit(w);
    std::cout << "All tasks are completed." << std::endl;
    return 0;
}
