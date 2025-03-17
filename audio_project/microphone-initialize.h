#pragma once
#include <vector>

// Initialize the microphone
bool initMicrophone();

// Start recording (blocking mode), capture a short audio data segment
// Return the captured audio data (bytes or sample buffer of PCM)
std::vector<float> recordAudio(size_t numSamples, int sampleRate, int channels);
