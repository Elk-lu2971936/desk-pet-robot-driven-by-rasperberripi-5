#include <iostream>
#include <fstream>
#include <cstdint>
#include <cmath>
#include <string>
#include "voice_max_loudness.h"
#include "debug.h"


struct WAVHeader {
    char      riff[4];         
    uint32_t  chunkSize;       
    char      wave[4];         
    char      fmt[4];          
    uint32_t  subchunk1Size;   
    uint16_t  audioFormat;     
    uint16_t  numChannels;     
    uint32_t  sampleRate;      
    uint32_t  byteRate;        
    uint16_t  blockAlign;     
    uint16_t  bitsPerSample;   
    char      data[4];         
    uint32_t  subchunk2Size;   
};


double get_max_loudness(const char * filename) 
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        LOG_ERR("Failed to open WAV file.");
        return size_t(-1);
    }

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (header.audioFormat != 1 || header.bitsPerSample != 16) {
        LOG_ERR("Only 16-bit PCM WAV files are supported.");
        return size_t(-1);
    }

    size_t numSamples = header.subchunk2Size / sizeof(int16_t);

    int16_t* samples = new int16_t[numSamples];

    
    file.read(reinterpret_cast<char*>(samples), header.subchunk2Size);
    file.close();

    
    int16_t maxSample = 0;
    for (size_t i = 0; i < numSamples; ++i) {
        if (std::abs(samples[i]) > std::abs(maxSample)) {
            maxSample = samples[i];
        }
    }


    double peakLoudness = maxSample / 32768.0 * 100.0;  

    
    double peakLoudnessDB = 20 * std::log10(std::abs(maxSample) / 32768.0);
    LOG_DBG("Maximum peak loudness (dB): %f dB", peakLoudnessDB);

    delete[] samples;
    
    return peakLoudnessDB;
}

#ifdef _XTEST

int main() {
    std::string filePath = "example.wav";
    double maxLoudness = get_max_loudness(filePath.c_str());

    if (maxLoudness != -1) {
        std::cout << "Maximum peak loudness (percentage): " << maxLoudness << "%" << std::endl;
    }

    return 0;
}
#endif//_XTEST
