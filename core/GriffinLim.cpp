#include "core/GriffinLim.h"
#include "core/Stft.h"
#include <random>
#include <iostream>

namespace img2spec {

GriffinLim::GriffinLim() {}
GriffinLim::~GriffinLim() {}

void GriffinLim::initializeRandomPhase(
    std::vector<std::vector<float>>& phase,
    int numFrames,
    int numBins
) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 2.0f * M_PI);

    phase.resize(numFrames);
    for (int t = 0; t < numFrames; ++t) {
        phase[t].resize(numBins);
        for (int k = 0; k < numBins; ++k) {
            phase[t][k] = dist(gen);
        }
    }
}

std::vector<float> GriffinLim::reconstruct(
    const std::vector<std::vector<float>>& magnitudeSpectrogram,
    Stft& stft,
    int numIterations,
    ProgressCallback progressCallback,
    bool* cancelFlag
) {
    if (magnitudeSpectrogram.empty()) {
        std::cerr << "GriffinLim: Empty magnitude spectrogram" << std::endl;
        return {};
    }

    const int numFrames = magnitudeSpectrogram.size();
    const int numBins = magnitudeSpectrogram[0].size();

    std::cout << "GriffinLim: Starting reconstruction" << std::endl;
    std::cout << "  Frames: " << numFrames << ", Bins: " << numBins << std::endl;
    std::cout << "  Iterations: " << numIterations << std::endl;

    // Initialize random phase
    std::vector<std::vector<float>> phase;
    initializeRandomPhase(phase, numFrames, numBins);

    // Create complex spectrogram from magnitude + phase
    std::vector<std::vector<std::complex<float>>> complexSpec(numFrames);
    for (int t = 0; t < numFrames; ++t) {
        complexSpec[t].resize(numBins);
        for (int k = 0; k < numBins; ++k) {
            const float mag = magnitudeSpectrogram[t][k];
            const float ph = phase[t][k];
            complexSpec[t][k] = std::polar(mag, ph);
        }
    }

    // Griffin-Lim iterations
    std::vector<float> audio;
    for (int iter = 0; iter < numIterations; ++iter) {
        // Check for cancellation
        if (cancelFlag && *cancelFlag) {
            std::cout << "GriffinLim: Cancelled at iteration " << iter << std::endl;
            break;
        }

        // 1) ISTFT: complex spectrogram -> time-domain signal
        audio = stft.inverse(complexSpec);

        // 2) STFT: time-domain signal -> complex spectrogram
        auto newSpec = stft.forward(audio);

        // 3) Extract phase, but keep original magnitude
        for (int t = 0; t < numFrames && t < static_cast<int>(newSpec.size()); ++t) {
            for (int k = 0; k < numBins && k < static_cast<int>(newSpec[t].size()); ++k) {
                const float newPhase = std::arg(newSpec[t][k]);
                const float origMag = magnitudeSpectrogram[t][k];
                complexSpec[t][k] = std::polar(origMag, newPhase);
            }
        }

        // Progress callback
        if (progressCallback) {
            progressCallback(iter + 1, numIterations);
        }

        // Log progress every 10 iterations
        if ((iter + 1) % 10 == 0 || iter == 0 || iter == numIterations - 1) {
            std::cout << "  Iteration " << (iter + 1) << "/" << numIterations << std::endl;
        }
    }

    // Final ISTFT
    audio = stft.inverse(complexSpec);

    std::cout << "GriffinLim: Reconstruction complete. Output length: " << audio.size() << " samples" << std::endl;

    return audio;
}

} // namespace img2spec
