#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <limits>
#include <cassert>

// =============================================================
// 1) WAV 读取 (16-bit单声道)
// =============================================================
struct WavData {
    int sampleRate;
    std::vector<double> samples; // 存放音频数据(浮点)
};

bool readMonoWav16(const std::string &filename, WavData &outData) {
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open()) {
        std::cerr << "Cannot open wav file: " << filename << std::endl;
        return false;
    }

    // 简易地跳过 RIFF 头并读取格式，假设必然是 16-bit 单声道
    char riff[4];
    ifs.read(riff, 4); // "RIFF"
    if (!ifs.good()) { std::cerr << "Invalid WAV header.\n"; return false; }
    uint32_t chunkSize;
    ifs.read(reinterpret_cast<char*>(&chunkSize), 4);
    char wave[4];
    ifs.read(wave, 4); // "WAVE"

    // 查找 "fmt " subchunk
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

    // 跳过可能有的 "extra param"
    if (subchunk1Size > 16) {
        ifs.ignore(subchunk1Size - 16);
    }

    // 找到 "data" subchunk
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

    // 读音频数据
    outData.sampleRate = sampleRate;
    size_t numSamples = dataSize / (bitsPerSample/8) / numChannels;
    outData.samples.resize(numSamples);

    for (size_t i = 0; i < numSamples; i++) {
        int16_t tmp;
        ifs.read(reinterpret_cast<char*>(&tmp), 2);
        // 只取单声道 (numChannels=1 假设)
        outData.samples[i] = static_cast<double>(tmp) / 32768.0;
    }
    return true;
}

// =============================================================
// 2) 简易MFCC提取
//    - 分帧(25ms, 10ms移)
//    - Hamming窗
//    - 计算能量谱
//    - Mel滤波(简化), 取26个滤波器
//    - 对数/ DCT 取前13个系数
// =============================================================

// 计算汉明窗
inline double hamming(size_t n, size_t N) {
    return 0.54 - 0.46 * cos((2.0 * M_PI * n) / (N - 1));
}

// 简易 FFT 用于得到幅度谱(可以换成更高效的库)
static void naiveDFT(const std::vector<double> &in, std::vector<double> &outReal, std::vector<double> &outImag) {
    // out[k] = sum_{n=0..N-1} in[n]* exp(-j2pi k n / N)
    // 这里只做非常朴素的 O(N^2)
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

// Mel -> 频率
inline double melToHz(double mel) {
    return 700.0 * (exp(mel / 1127.0) - 1.0);
}
// 频率 -> Mel
inline double hzToMel(double hz) {
    return 1127.0 * log(1.0 + hz / 700.0);
}

// 简单 DCT 用于倒谱
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
    // 1) 分帧: 25ms, 10ms移
    int frameLen = (int)(0.025 * sampleRate); // 25ms
    int frameShift = (int)(0.01 * sampleRate); // 10ms
    // 这里做最简单的: 不做预加重, 不做端点检测

    std::vector<std::vector<double>> mfccResult;

    // Mel 滤波器数量
    const int nMel = 26;
    // MFCC取前13个
    const int nMFCC = 13;

    // 计算Mel滤波器的边界
    double melMin = hzToMel(0.0);
    double melMax = hzToMel(sampleRate / 2.0);
    std::vector<double> melPoints(nMel+2);
    for (int i = 0; i < nMel+2; i++) {
        melPoints[i] = melMin + (melMax - melMin) * i / (nMel+1);
    }
    // 转成Hz
    std::vector<double> binHz(nMel+2);
    for (int i = 0; i < (int)binHz.size(); i++) {
        binHz[i] = melToHz(melPoints[i]);
    }
    // 转成FFT bin 索引(假设frameLen点FFT)
    // 只演示: 直接对 frameLen 做 naiveDFT
    // 实际中通常是下一个2^N大小
    auto freqToBin = [&](double freq){
        return (int)std::floor((frameLen)* freq / sampleRate);
    };
    std::vector<int> binIndices(nMel+2);
    for (int i = 0; i < (int)binIndices.size(); i++) {
        binIndices[i] = freqToBin(binHz[i]);
    }

    for (int start = 0; start + frameLen <= (int)audio.size(); start += frameShift) {
        // 2) 取出一帧并加窗
        std::vector<double> frame(frameLen);
        for (int i = 0; i < frameLen; i++) {
            frame[i] = audio[start + i] * hamming(i, frameLen);
        }

        // 3) DFT -> 幅度谱
        std::vector<double> re, im;
        naiveDFT(frame, re, im);
        // 只取 0..frameLen/2
        int half = frameLen/2;
        std::vector<double> powerSpec(half+1);
        for (int k = 0; k <= half; k++) {
            double mag = re[k]*re[k] + im[k]*im[k];
            powerSpec[k] = mag; // 或者再除以 frameLen
        }

        // 4) Mel滤波 -> 26维
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
        // 对数
        for (int m = 0; m < nMel; m++) {
            melEnergies[m] = std::log10(std::max(melEnergies[m], 1e-10));
        }
        // 5) DCT -> 取前13
        // 这里把 melEnergies.size()=26 => 进行26点DCT, 返回26维
        auto dctOut = dct(melEnergies);
        dctOut.resize(nMFCC); // 截取前13
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
// 主函数：
// usage: ./minimal ref1.wav label1 ref2.wav label2 ref3.wav label3 ref4.wav label4 input.wav
// =============================================================
int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage example:\n"
                  << "  ./minimal stand_up.wav stand_up forward.wav forward back.wav back sit_down.wav sit_down input.wav\n";
        return 1;
    }

    // 解析命令行, 最后一个参数是输入音频, 之前的是 pairs(refWav, label)
    // 假设我们要加载N对： (ref.wav, label), 最后一条是 input.wav
    // 例如: ArgCount = 9 => (4对reference + 1输入)
    // Layout: [RefWav Label], [RefWav Label], ..., [InputWav]

    int numRefs = (argc - 2) / 2;
    // example: if argc=9 => (9-2)/2=3.5 => 3 pairs => indices=1~6 => last index=7 => input

    std::vector<std::string> refFiles, refLabels;
    for (int i = 1; i < 2*numRefs; i += 2) {
        refFiles.push_back(argv[i]);
        refLabels.push_back(argv[i+1]);
    }
    std::string inputFile = argv[2*numRefs+1];

    // 读取参考音频并提取MFCC
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

    // 读取输入音频
    std::cout << "Loading input: " << inputFile << std::endl;
    WavData inWav;
    if (!readMonoWav16(inputFile, inWav)) {
        return 1;
    }
    auto inMfcc = computeMFCC(inWav.samples, inWav.sampleRate);
    std::cout << "  input frames=" << inMfcc.size() << std::endl;

    // DTW匹配
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

    // 设置个简单阈值
    if (bestDist > 1800.0) {
        std::cout << "Result: Unrecognized (distance " << bestDist << ")\n";
    } else {
        std::cout << "Result: " << bestLabel << " (distance " << bestDist << ")\n";
    }

    return 0;
}
