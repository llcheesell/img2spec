#pragma once

#include <vector>
#include <string>

namespace img2spec {

enum class FrequencyScale {
    Linear,
    Logarithmic
};

struct SpectrogramParams {
    int fftSize = 2048;
    int hopSize = 512;
    FrequencyScale freqScale = FrequencyScale::Linear;
    double minDb = -80.0;
    double gamma = 1.0;
};

class SpectrogramBuilder {
public:
    SpectrogramBuilder();
    ~SpectrogramBuilder();

    // Build magnitude spectrogram from grayscale image data
    // Returns: vector of frames, each frame has (fftSize/2+1) bins
    std::vector<std::vector<float>> buildMagnitudeSpectrogram(
        const std::vector<float>& imageData,
        int imageWidth,
        int imageHeight,
        const SpectrogramParams& params
    );

private:
    float mapPixelToMagnitude(float pixel, double minDb, double gamma);
    std::vector<float> resampleColumn(
        const std::vector<float>& imageData,
        int imageWidth,
        int imageHeight,
        int frameIndex,
        int numBins,
        FrequencyScale scale
    );
};

} // namespace img2spec
