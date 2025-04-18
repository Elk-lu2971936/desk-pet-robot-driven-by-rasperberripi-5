#include "debug.h"
#include "whisper_fuzzy.h"
#include "rotate180.h"   // 包含旋转舵机接口头文件
#include "oled_display.h" // 新增OLED显示接口

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>

/**
 * Whisper 识别回调函数
 * 当匹配到：
 *   - code="0x06" 时，调用 rotateServo180() 使舵机旋转到 180°（standup），同时显示 ⊙▽⊙ 表情；
 *   - code="0x05" 时，调用 rotateServo0() 使舵机归位到 0°（sleep 模式），同时显示 (￣_,￣ ) 表情；
 *   - code="0x00" 时，交替执行：
 *         先使 GPIO12（舵机从 180° 到 90°）执行一次“向前”旋转，
 *         再使 GPIO13（舵机从 0° 到 180°）执行一次“向后”旋转，
 *         共循环 6 个周期。
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
        std::cout << "[Action] Matched code 0x06 => stand up: rotating both servos to 180° and displaying ⊙▽⊙" << std::endl;
        showStandUp();   // 显示 stand up 表情
        rotateServo180();
    } else if (strcmp(code, "0x05") == 0) {
        std::cout << "[Action] Matched code 0x05 => sleep: rotating both servos to 0° and displaying (￣_,￣ )" << std::endl;
        showSleep();     // 显示 sleep 表情
        rotateServo0();
    } else if (strcmp(code, "0x00") == 0) {
        std::cout << "[Action] Matched code 0x00 => performing alternating rotations:" << std::endl;
        alternateRotation();
    }
    return 0;
}

/**
 * Whisper 任务线程
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
 * 主函数
 */
int main(int argc, char const* argv[]) {
    // 初始化OLED显示
    initOLED();

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
