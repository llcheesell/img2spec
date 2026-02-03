#pragma once

#include <vector>
#include <functional>

namespace img2spec {

class Stft;

using ProgressCallback = std::function<void(int current, int total)>;

class GriffinLim {
public:
    GriffinLim();
    ~GriffinLim();

    // Reconstruct audio from magnitude spectrogram using Griffin-Lim algorithm
    // Returns: time-domain audio signal
    std::vector<float> reconstruct(
        const std::vector<std::vector<float>>& magnitudeSpectrogram,
        Stft& stft,
        int numIterations,
        ProgressCallback progressCallback = nullptr,
        bool* cancelFlag = nullptr
    );

private:
    void initializeRandomPhase(
        std::vector<std::vector<float>>& phase,
        int numFrames,
        int numBins
    );
};

} // namespace img2spec
