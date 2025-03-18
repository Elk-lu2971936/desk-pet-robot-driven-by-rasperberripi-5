#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <limits>
#include <cassert>

// =============================================================
// 1) WAV read (16-bit mono)
// =============================================================
struct WavData {
    int sampleRate;
    std::vector<double> samples; //Store audio data (floating point)
};

bool readMonoWav16(const std::string &filename, WavData &outData) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Cannot open wav file: " << filename << std::endl;
        return false;
    }

   // Simply skip the RIFF header and read the format, assuming it must be 16-bit mono
    char riff[4];
    ifs.read(riff, 4); // "RIFF"
    if (!ifs.good()) { std::cerr << "Invalid WAV header.\n"; return false; }
    uint32_t chunkSize;
    ifs.read(reinterpret_cast<char*>(&chunkSize), 4);
    char wave[4];
    ifs.read(wave, 4); // "WAVE"

   // Search "fmt " subchunk
    char fmt[4];
    uint32_t subchunk1Size;
    ifs.read(fmt, 4); // "fmt "
    ifs.read(reinterpret_cast<char*>(&subchunk1Size), 4);
    uint16_t audioFormat, numChannels;
    uint32_t sampleRate, byteRate;
    uint16_t blockAlign, bitsPerSample;

    ifs.read(reinterpret_cast<char*>(&audioFormat), 2);
    ifs.read(reinterpret_cast<char*>(&numChannels), 2);
    ifs.read(reinterpret_cast<char*>(&sampleRate), 4);
    ifs.read(reinterpret_cast<char*>(&byteRate), 4);
    ifs.read(reinterpret_cast<char*>(&blockAlign), 2);
    ifs.read(reinterpret_cast<char*>(&bitsPerSample), 2);

   // Skip possible "extra param"
    if (subchunk1Size > 16) {
        ifs.ignore(subchunk1Size - 16);
    }

// Find the "data" subchunk
    char dataHeader[4];
    uint32_t dataSize = 0;
    while (true) {
        ifs.read(dataHeader, 4);
        if (!ifs.good()) {
            std::cerr << "Cannot find data subchunk.\n";
            return false;
        }
        ifs.read(reinterpret_cast<char*>(&dataSize), 4);
        if (std::string(dataHeader,4) == "data") {
            break; // found "data"
        } else {
            // skip
            ifs.ignore(dataSize);
        }
    }

  // Read audio data
    outData.sampleRate = sampleRate;
    size_t numSamples = dataSize / (bitsPerSample/8) / numChannels;
    outData.samples.resize(numSamples);

    for (size_t i = 0; i < numSamples; i++) {
        int16_t tmp;
        ifs.read(reinterpret_cast<char*>(&tmp), 2);
// Only take single channel (numChannels=1 assumption)
        outData.samples[i] = static_cast<double>(tmp) / 32768.0;
    }
    return true;
}

// =============================================================
// 2) Simple MFCC extraction
// - Frame division (25ms, 10ms shift)
// - Hamming window
// - Calculate energy spectrum
// - Mel filter (simplified), take 26 filters
// - Logarithm / DCT take the first 13 coefficients
// =============================================================

// Calculate the Hamming window
inline double hamming(size_t n, size_t N) {
    return 0.54 - 0.46 * cos((2.0 * M_PI * n) / (N - 1));
}

// Simple FFT is used to obtain the amplitude spectrum (can be replaced with a more efficient library)
static void naiveDFT(const std::vector<double> &in, std::vector<double> &outReal, std::vector<double> &outImag) {
    // out[k] = sum_{n=0..N-1} in[n]* exp(-j2pi k n / N)
// Here we only do a very simple O(N^2)
    int N = (int)in.size();
    outReal.resize(N);
    outImag.resize(N);
    for (int k = 0; k < N; k++) {
        double realSum = 0.0, imagSum = 0.0;
        for (int n = 0; n < N; n++) {
            double theta = -2.0 * M_PI * k * n / N;
            realSum += in[n] * cos(theta);
            imagSum += in[n] * sin(theta);
        }
        outReal[k] = realSum;
        outImag[k] = imagSum;
    }
}

// Mel -> frequency
inline double melToHz(double mel) {
    return 700.0 * (exp(mel / 1127.0) - 1.0);
}
// frequency -> Mel
inline double hzToMel(double hz) {
    return 1127.0 * log(1.0 + hz / 700.0);
}

// Simple DCT for cepstrum
static std::vector<double> dct(const std::vector<double> &in) {
    int N = (int)in.size();
    std::vector<double> out(N, 0.0);
    for (int k = 0; k < N; k++) {
        double sum = 0.0;
        for (int n = 0; n < N; n++) {
            sum += in[n] * cos(M_PI * k * (2.0 * n + 1.0) / (2.0 * N));
        }
        out[k] = sum;
    }
    return out;
}

std::vector<std::vector<double>> computeMFCC(const std::vector<double> &audio, int sampleRate) {
// 1) Frame: 25ms, 10ms shift
    int frameLen = (int)(0.025 * sampleRate); // 25ms
    int frameShift = (int)(0.01 * sampleRate); // 10ms
// Here is the simplest one: no pre-emphasis, no endpoint detection

    std::vector<std::vector<double>> mfccResult;

//Number of Mel filters
    const int nMel = 26;
// Take the first 13 MFCCs
    const int nMFCC = 13;

// Calculate the boundaries of the Mel filter
    double melMin = hzToMel(0.0);
    double melMax = hzToMel(sampleRate / 2.0);
    std::vector<double> melPoints(nMel+2);
    for (int i = 0; i < nMel+2; i++) {
        melPoints[i] = melMin + (melMax - melMin) * i / (nMel+1);
    }
// Convert to Hz
    std::vector<double> binHz(nMel+2);
    for (int i = 0; i < (int)binHz.size(); i++) {
        binHz[i] = melToHz(melPoints[i]);
    }
// Convert to FFT bin index (assuming frameLen point FFT)
// Demonstration only: do naiveDFT directly on frameLen
// In practice, it is usually the next 2^N size
    auto freqToBin = [&](double freq){
        return (int)std::floor((frameLen)* freq / sampleRate);
    };
    std::vector<int> binIndices(nMel+2);
    for (int i = 0; i < (int)binIndices.size(); i++) {
        binIndices[i] = freqToBin(binHz[i]);
    }

    for (int start = 0; start + frameLen <= (int)audio.size(); start += frameShift) {
// 2) Take out a frame and add a window
        std::vector<double> frame(frameLen);
        for (int i = 0; i < frameLen; i++) {
            frame[i] = audio[start + i] * hamming(i, frameLen);
        }

// 3) DFT -> Amplitude Spectrum
        std::vector<double> re, im;
        naiveDFT(frame, re, im);
// Only take 0..frameLen/2
        int half = frameLen/2;
        std::vector<double> powerSpec(half+1);
        for (int k = 0; k <= half; k++) {
            double mag = re[k]*re[k] + im[k]*im[k];
            powerSpec[k] = mag; // Or divide by frameLen
        }

// 4) Mel filter -> 26 dimensions
        std::vector<double> melEnergies(nMel, 0.0);
        for (int m = 1; m <= nMel; m++) {
            int startBin = binIndices[m-1];
            int peakBin  = binIndices[m];
            int endBin   = binIndices[m+1];
            for (int k = startBin; k <= endBin; k++) {
                if (k < 0 || k > half) continue;
                double weight = 0.0;
                if (k < peakBin)
                    weight = (k - startBin) / double(peakBin - startBin);
                else
                    weight = (endBin - k) / double(endBin - peakBin);
                if (weight < 0) weight = 0;
                melEnergies[m-1] += powerSpec[k] * weight;
            }
        }
 // Logarithm
        for (int m = 0; m < nMel; m++) {
            melEnergies[m] = std::log10(std::max(melEnergies[m], 1e-10));
        }
// 5) DCT -> take the first 13
        // Here melEnergies.size()=26 => perform 26-point DCT, return 26-dimensional
        auto dctOut = dct(melEnergies);
        dctOut.resize(nMFCC); // Intercept the first 13
        mfccResult.push_back(dctOut);
    }
    return mfccResult;
}

// =============================================================
// 3) DTW
// =============================================================
double dtwDistance(const std::vector<std::vector<double>> &seq1,
                   const std::vector<std::vector<double>> &seq2)
{
    int n = (int)seq1.size();
    int m = (int)seq2.size();
    if (n == 0 || m == 0) {
        return std::numeric_limits<double>::infinity();
    }
    std::vector<std::vector<double>> dp(n+1, std::vector<double>(m+1, std::numeric_limits<double>::infinity()));
    dp[0][0] = 0.0;

    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= m; j++) {
            // 计算帧i-1与帧j-1之间的欧氏距离
            assert(seq1[i-1].size() == seq2[j-1].size());
            double cost = 0.0;
            for (size_t k = 0; k < seq1[i-1].size(); k++) {
                double diff = seq1[i-1][k] - seq2[j-1][k];
                cost += diff * diff;
            }
            cost = std::sqrt(cost);
            dp[i][j] = cost + std::min({dp[i-1][j], dp[i][j-1], dp[i-1][j-1]});
        }
    }
    return dp[n][m];
}

// =============================================================
// Main function:
// usage: ./minimal ref1.wav label1 ref2.wav label2 ref3.wav label3 ref4.wav label4 input.wav
// =============================================================
int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage example:\n"
                  << "  ./minimal stand_up.wav stand_up forward.wav forward back.wav back sit_down.wav sit_down input.wav\n";
        return 1;
    }

// Parse the command line, the last parameter is the input audio, the previous one is pairs(refWav, label)
// Assume we want to load N pairs: (ref.wav, label), the last one is input.wav
// For example: ArgCount = 9 => (4 reference pairs + 1 input)
// Layout: [RefWav Label], [RefWav Label], ..., [InputWav]

    int numRefs = (argc - 2) / 2;
    // example: if argc=9 => (9-2)/2=3.5 => 3 pairs => indices=1~6 => last index=7 => input

    std::vector<std::string> refFiles, refLabels;
    for (int i = 1; i < 2*numRefs; i += 2) {
        refFiles.push_back(argv[i]);
        refLabels.push_back(argv[i+1]);
    }
    std::string inputFile = argv[2*numRefs+1];

   // Read reference audio and extract MFCC
    std::vector<std::vector<std::vector<double>>> refMfccs(numRefs);
    std::cout << "Loading references...\n";
    for (int i = 0; i < numRefs; i++) {
        WavData wd;
        if (!readMonoWav16(refFiles[i], wd)) {
            return 1;
        }
        auto mfcc = computeMFCC(wd.samples, wd.sampleRate);
        refMfccs[i] = mfcc;
        std::cout << "  Loaded \"" << refFiles[i] << "\" => label \"" << refLabels[i]
                  << "\", frames=" << mfcc.size() << std::endl;
    }

    // Read input audio
    std::cout << "Loading input: " << inputFile << std::endl;
    WavData inWav;
    if (!readMonoWav16(inputFile, inWav)) {
        return 1;
    }
    auto inMfcc = computeMFCC(inWav.samples, inWav.sampleRate);
    std::cout << "  input frames=" << inMfcc.size() << std::endl;

   // DTW matching
    double bestDist = std::numeric_limits<double>::infinity();
    std::string bestLabel = "Unrecognized";
    for (int i = 0; i < numRefs; i++) {
        double dist = dtwDistance(inMfcc, refMfccs[i]);
        std::cout << "  DTW to [" << refLabels[i] << "]: " << dist << std::endl;
        if (dist < bestDist) {
            bestDist = dist;
            bestLabel = refLabels[i];
        }
    }

    // Set a simple threshold
    if (bestDist > 1800.0) {
        std::cout << "Result: Unrecognized (distance " << bestDist << ")\n";
    } else {
        std::cout << "Result: " << bestLabel << " (distance " << bestDist << ")\n";
    }

    return 0;
}
