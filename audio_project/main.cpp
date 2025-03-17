#include <iostream>
#include <vector>
#include <string>

#include "microphoneInitialize.h"
#include "whisper.h"

int main() {
    if (!initMicrophone()) {
        std::cerr << "Microphone init failed.\n";
        return 1;
    }

    const int sampleRate = 16000; // Whisper  16k
    const int channels   = 1;  
    size_t numSamples = sampleRate * 2;

    std::cout << "Recording...\n";
    std::vector<float> audioData = recordAudio(numSamples, sampleRate, channels);
    if (audioData.empty()) {
        std::cerr << "No audio captured!\n";
        return 1;
    }
    std::cout << "Recording done.\n";

    // 3) Whisper：loading model
    const char *modelPath = "ggml-small.en.bin"; 
    struct whisper_context *ctx = whisper_init_from_file(modelPath);
    if (!ctx) {
        std::cerr << "Failed to load Whisper model: " << modelPath << "\n";
        return 1;
    }


    whisper_full_params wparams = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    wparams.print_progress   = false;
    wparams.print_realtime   = false;
    wparams.print_timestamps = false;
    wparams.language         = "en"; 

    int ret = whisper_full(ctx, wparams, audioData.data(), audioData.size());
    if (ret != 0) {
        std::cerr << "whisper_full() failed: " << ret << "\n";
        whisper_free(ctx);
        return 1;
    }

    const int n_segments = whisper_full_n_segments(ctx);
    std::cout << "Recognized text: ";
    for (int i = 0; i < n_segments; i++) {
        const char *text = whisper_full_get_segment_text(ctx, i);
        std::cout << text;
    }
    std::cout << std::endl;

    whisper_free(ctx);


    return 0;
}
