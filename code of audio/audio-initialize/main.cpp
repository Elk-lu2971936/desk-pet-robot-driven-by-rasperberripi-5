#include "debug.h"
#include "whisper.h"
#include <iostream>
#include <thread>
#include <vector>


/**
 * Whisper 
 *
 * @param text 
 * @param code 
 * @param userdata 
 */
void whisper_callback(const char *text, const char* code, void* userdata)
{
    size_t *count = (size_t *)userdata;
    // LOG_DBG("[%zu] get code: %s", *count, code);
    std::cout << "[" << *count << "] " << "get text: '" << text << "' code: " << code << std::endl;
}

// 
void whisper_task(void* userdata)
{
    whisper_t* w = (whisper_t*)userdata;
    std::cout << "Task whisper is running on thread " << std::this_thread::get_id() << std::endl;
    size_t count = 0;

    while (1) {
        int ret = whisper(w, whisper_callback, &count);
        LOG_DBG("whisper ret: %d", ret);
        ++count;
    }

    std::cout << "Task whisper completed" << std::endl;
}

int main(int argc, char const* argv[])
{
    if (argc != 2) {
        LOG_ERR("use: %s config.json", argv[0]);
        return -1;
    }
    const int numThreads = 1; 
    std::vector<std::thread> threads;

    whisper_t* w = whisper_init(argv[1]);
    if (!w) {
        LOG_ERR("fail to init whisper");
        return -1;
    }

    
    for (int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(whisper_task, w)); 
    }

    
    for (auto& th : threads) {
        th.join(); 
    }
    whisper_exit(w);

    std::cout << "All tasks are completed." << std::endl;

    return 0;
}
