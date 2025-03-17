#include "microphoneInitialize.h"
#include <portaudio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <mutex>
#include <condition_variable>

static PaStream* g_stream = nullptr;

struct RecordingData {
    std::vector<float>* buffer;
    std::size_t maxSamples;
    std::size_t channels;
    std::size_t writeIndex;
    std::mutex mtx;
    std::condition_variable cv;
    bool finished;
};

// callback function
static int audioCallback(const void* input,
                         void* output,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void* userData)
{
    RecordingData* data = reinterpret_cast<RecordingData*>(userData);
    if (input == nullptr) {
        return paContinue;
    }

    const float* in = static_cast<const float*>(input);
    std::size_t framesToCopy = framesPerBuffer;
    std::size_t framesNeeded = data->maxSamples - data->writeIndex;
    if (framesToCopy > framesNeeded) {
        framesToCopy = framesNeeded;
    }

    std::size_t samplesToCopy = framesToCopy * data->channels;
    std::size_t startPos = data->writeIndex * data->channels;

    for (std::size_t i = 0; i < samplesToCopy; i++) {
        (*data->buffer)[startPos + i] = in[i];
    }
    data->writeIndex += framesToCopy;

    if (data->writeIndex >= data->maxSamples) {
        {
            std::lock_guard<std::mutex> lock(data->mtx);
            data->finished = true;
        }
        data->cv.notify_one();
        return paComplete; // Recording ends
    }
    return paContinue;     // Continue recording
}

bool initMicrophone() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    std::cout << "PortAudio initialized successfully." << std::endl;
    return true;
}

std::vector<float> recordAudio(std::size_t numSamples, int sampleRate, int channels) {
    // Allocate floating point buffer
    std::vector<float> recordedData(numSamples * channels, 0.0f);

    // Prepare user data for the callback
    RecordingData userData;
    userData.buffer = &recordedData;
    userData.maxSamples = numSamples;
    userData.channels = channels;
    userData.writeIndex = 0;
    userData.finished = false;

    // Set the input stream parameters
    PaStreamParameters inputParameters;
    std::memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "No default input device found." << std::endl;
        return {};
    }
    inputParameters.channelCount = channels;
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency =
        Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

   // Open the input stream in callback mode
    PaError err = Pa_OpenStream(
        &g_stream,
        &inputParameters,
        nullptr,
        sampleRate,
        256,         // framesPerBuffer
        paClipOff,
        audioCallback,
        &userData
    );
    if (err != paNoError) {
        std::cerr << "Pa_OpenStream error: " << Pa_GetErrorText(err) << std::endl;
        return {};
    }

    // Start the stream
    err = Pa_StartStream(g_stream);
    if (err != paNoError) {
        std::cerr << "Pa_StartStream error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(g_stream);
        g_stream = nullptr;
        return {};
    }

    // Wait for recording to complete (via condition variable)
    {
        std::unique_lock<std::mutex> lock(userData.mtx);
        userData.cv.wait(lock, [&userData] {
            return userData.finished;
        });
    }

    //After recording is finished, manually stop and close the stream
    Pa_StopStream(g_stream);
    Pa_CloseStream(g_stream);
    g_stream = nullptr;

    std::cout << "Captured (callback) " << numSamples
              << " samples (per channel)." << std::endl;
    return recordedData;
}

