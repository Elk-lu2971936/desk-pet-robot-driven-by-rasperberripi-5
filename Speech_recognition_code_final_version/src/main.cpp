#include "debug.h"
#include "whisper_fuzzy.h"
#include <iostream>
#include <thread>
#include <vector>



static int whisper_user_callback(size_t leat_count, const char *text, const char* code, void* userdata)
{
    // skip.
    if (leat_count) {
        return 1;
    }
    if (!text || !code || !userdata) {
        LOG_ERR("args fail! text(%p), code(%p), userdata(%p)", text, code, userdata);
        return -1;
    }
    size_t &count = *(size_t *)userdata;
    ++count;

    LOG_INFO("[%zu] get text: %s, code: %s", count, text, code);

    return 0;
}

void whisper_fuzzy_task(void* userdata)
{
    whisper_fuzzy_t* w = (whisper_fuzzy_t*)userdata;
    std::cout << "Task whisper is running on thread " << std::this_thread::get_id() << std::endl;
    size_t count = 0;

    int ret = whisper_fuzzy(w, whisper_user_callback, &count);
    LOG_INFO("whisper ret: %d", ret);

    std::cout << "Task whisper completed" << std::endl;
}

int main(int argc, char const* argv[])
{
    const int numThreads = 1; 
    std::vector<std::thread> threads;

    whisper_fuzzy_t* w = whisper_fuzzy_init(argc, argv);
    if (!w) {
        LOG_ERR("fail to init whisper");
        return -1;
    }


    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(whisper_fuzzy_task, w)); 
    }


    for (auto& th : threads) {
        th.join(); 
    }

    std::cout << "clear fuzzy." << std::endl;

    whisper_fuzzy_exit(w);

    std::cout << "All tasks are completed." << std::endl;

    return 0;
}

