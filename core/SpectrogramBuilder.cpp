#include "core/SpectrogramBuilder.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace img2spec {

SpectrogramBuilder::SpectrogramBuilder() {}
SpectrogramBuilder::~SpectrogramBuilder() {}

float SpectrogramBuilder::mapPixelToMagnitude(float pixel, double minDb, double gamma) {
    // Apply gamma correction
    float p = std::pow(pixel, static_cast<float>(gamma));

    // Map to dB range [minDb, 0]
    float magDb = minDb + p * (-minDb);

    // Convert dB to linear magnitude
    float mag = std::pow(10.0f, magDb / 20.0f);

    return mag;
}

std::vector<float> SpectrogramBuilder::resampleColumn(
    const std::vector<float>& imageData,
    int imageWidth,
    int imageHeight,
    int frameIndex,
    int numBins,
    FrequencyScale scale
) {
    std::vector<float> column(numBins);

    if (frameIndex >= imageWidth) {
        std::fill(column.begin(), column.end(), 0.0f);
        return column;
    }

    // Image Y axis maps to frequency bins
    // Note: Image convention is Y=0 at top, but we want high frequencies at top
    // So we flip: imageY=0 → high freq, imageY=(height-1) → low freq (DC)

    if (scale == FrequencyScale::Linear) {
        // Linear frequency mapping
        for (int k = 0; k < numBins; ++k) {
            // Map bin k to image Y coordinate linearly
            // k=0 (DC) → bottom of image (imageY = height-1)
            // k=numBins-1 (Nyquist) → top of image (imageY = 0)
            const float imageY = (imageHeight - 1) * (1.0f - static_cast<float>(k) / (numBins - 1));

            // Bilinear interpolation
            const int y0 = static_cast<int>(std::floor(imageY));
            const int y1 = std::min(y0 + 1, imageHeight - 1);
            const float fy = imageY - y0;

            const float v0 = imageData[y0 * imageWidth + frameIndex];
            const float v1 = imageData[y1 * imageWidth + frameIndex];

            column[k] = v0 * (1.0f - fy) + v1 * fy;
        }
    } else {
        // Logarithmic frequency mapping
        // Use perceptual scale: more resolution in low frequencies
        // Map image Y to log frequency, then to bin index

        // Define frequency range (skip DC for log scale)
        const float minFreq = 20.0f; // Hz (lowest audible)
        const float maxFreq = 20000.0f; // Hz (Nyquist approximation)

        for (int k = 0; k < numBins; ++k) {
            if (k == 0) {
                // DC bin: always map to bottom of image
                const float imageY = imageHeight - 1;
                const int y = std::min(static_cast<int>(imageY), imageHeight - 1);
                column[k] = imageData[y * imageWidth + frameIndex];
            } else {
                // Logarithmic mapping for k >= 1
                // Frequency of bin k (linear)
                const float binFreq = static_cast<float>(k) / (numBins - 1) * maxFreq;

                // Map frequency to log scale [0, 1]
                const float logFreq = std::log(binFreq / minFreq) / std::log(maxFreq / minFreq);
                const float logFreqClamped = std::max(0.0f, std::min(1.0f, logFreq));

                // Map to image Y coordinate (0 = top = high freq, 1 = bottom = low freq)
                const float imageY = (imageHeight - 1) * (1.0f - logFreqClamped);

                // Bilinear interpolation
                const int y0 = static_cast<int>(std::floor(imageY));
                const int y1 = std::min(y0 + 1, imageHeight - 1);
                const float fy = imageY - y0;

                const float v0 = imageData[y0 * imageWidth + frameIndex];
                const float v1 = imageData[y1 * imageWidth + frameIndex];

                column[k] = v0 * (1.0f - fy) + v1 * fy;
            }
        }
    }

    return column;
}

std::vector<std::vector<float>> SpectrogramBuilder::buildMagnitudeSpectrogram(
    const std::vector<float>& imageData,
    int imageWidth,
    int imageHeight,
    const SpectrogramParams& params
) {
    const int numBins = params.fftSize / 2 + 1;
    const int numFrames = imageWidth;

    std::cout << "SpectrogramBuilder: Building magnitude spectrogram" << std::endl;
    std::cout << "  Image size: " << imageWidth << "x" << imageHeight << std::endl;
    std::cout << "  Output: " << numFrames << " frames x " << numBins << " bins" << std::endl;
    std::cout << "  Frequency scale: " << (params.freqScale == FrequencyScale::Linear ? "Linear" : "Logarithmic") << std::endl;
    std::cout << "  Min dB: " << params.minDb << ", Gamma: " << params.gamma << std::endl;

    std::vector<std::vector<float>> spectrogram(numFrames);

    for (int t = 0; t < numFrames; ++t) {
        // Get column from image (resample to numBins)
        std::vector<float> pixelColumn = resampleColumn(
            imageData, imageWidth, imageHeight, t, numBins, params.freqScale
        );

        // Convert pixels to magnitudes
        spectrogram[t].resize(numBins);
        for (int k = 0; k < numBins; ++k) {
            spectrogram[t][k] = mapPixelToMagnitude(pixelColumn[k], params.minDb, params.gamma);
        }
    }

    std::cout << "SpectrogramBuilder: Completed magnitude spectrogram" << std::endl;

    return spectrogram;
}

} // namespace img2spec
