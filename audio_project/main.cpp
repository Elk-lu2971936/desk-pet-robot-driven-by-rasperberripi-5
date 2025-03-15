#include <iostream>
#include "microphoneInitialize.h"
#include "whilsper.h"

int main() {
    // Initialize the microphone (placeholder function)
    if (!initMicrophone()) {
        std::cerr << "Failed to initialize microphone." << std::endl;
        return -1;
    }
    
    // recording data
    std::string capturedAudio = "PCM binary data (placeholder)";
    
    // call whilsper
    if (!whilsper(capturedAudio)) {
        std::cerr << "Whisper recognition failed." << std::endl;
    } else {
        std::cout << "Recognition done." << std::endl;
    }
    
    return 0;
}

