#include "microphoneInitialize.h"
#include <iostream>
#include <vector>
#include <cstring>  // for memset

static PaStream *g_stream = nullptr;

// Initializing PortAudio
bool initMicrophone() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio init error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    std::cout << "PortAudio initialized successfully." << std::endl;
    return true;
}

std::vector<float> recordAudio(size_t numSamples, int sampleRate, int channels) {
    std::vector<float> recordedData(numSamples * channels, 0.0f);

    // Input stream parameters
    PaStreamParameters inputParameters;
    memset(&inputParameters, 0, sizeof(inputParameters));
    inputParameters.device = Pa_GetDefaultInputDevice();
    if (inputParameters.device == paNoDevice) {
        std::cerr << "No default input device found." << std::endl;
        return {};
    }
    inputParameters.channelCount = channels;
    inputParameters.sampleFormat = paFloat32; // Use float 32-bit format
    inputParameters.suggestedLatency =
        Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;

    // Open the audio stream (blocking mode)
    PaError err = Pa_OpenStream(
        &g_stream,
        &inputParameters,    // Input parameters
        nullptr,             // No output required
        sampleRate,          // Sampling rate
        paFramesPerBufferUnspecified, // framesPerBuffer(Let PortAudio choose the appropriate size)
        paNoFlag,            // Here you can set some stream flags
        nullptr,             // The callback function is nullptr, indicating blocking mode
        nullptr              // userData of callback function
    );

    if (err != paNoError) {
        std::cerr << "Pa_OpenStream error: " << Pa_GetErrorText(err) << std::endl;
        return {};
    }

    // Start streaming
    err = Pa_StartStream(g_stream);
    if (err != paNoError) {
        std::cerr << "Pa_StartStream error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(g_stream);
        return {};
    }

    // Read audio data (blocking)
    err = Pa_ReadStream(g_stream, recordedData.data(), numSamples);
    if (err != paNoError) {
        std::cerr << "Pa_ReadStream error: " << Pa_GetErrorText(err) << std::endl;
        Pa_StopStream(g_stream);
        Pa_CloseStream(g_stream);
        return {};
    }

   // Stop and close the stream
    Pa_StopStream(g_stream);
    Pa_CloseStream(g_stream);
    g_stream = nullptr;

    std::cout << "Captured " << numSamples << " samples." << std::endl;
    return recordedData;
}
