#include "core/Stft.h"
#include <kiss_fftr.h>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace img2spec {

Stft::Stft(int fftSize, int hopSize)
    : fftSize_(fftSize)
    , hopSize_(hopSize)
{
    createWindow();
    std::cout << "Stft: Initialized with FFT size=" << fftSize << ", hop=" << hopSize << std::endl;
}

Stft::~Stft() {}

void Stft::createWindow() {
    window_.resize(fftSize_);
    // Hann window
    for (int i = 0; i < fftSize_; ++i) {
        window_[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / fftSize_));
    }
}

std::vector<std::vector<std::complex<float>>> Stft::forward(const std::vector<float>& signal) {
    const int numSamples = signal.size();
    const int numFrames = 1 + (numSamples - fftSize_) / hopSize_;
    const int numBins = fftSize_ / 2 + 1;

    std::vector<std::vector<std::complex<float>>> spectrogram(numFrames);

    // Allocate Kiss FFT configuration
    kiss_fftr_cfg fftCfg = kiss_fftr_alloc(fftSize_, 0, nullptr, nullptr);
    if (!fftCfg) {
        std::cerr << "Stft: Failed to allocate Kiss FFT configuration" << std::endl;
        return spectrogram;
    }

    std::vector<float> frame(fftSize_);
    std::vector<kiss_fft_cpx> fftOut(numBins);

    for (int t = 0; t < numFrames; ++t) {
        const int startIdx = t * hopSize_;

        // Extract and window frame
        for (int i = 0; i < fftSize_; ++i) {
            const int idx = startIdx + i;
            frame[i] = (idx < numSamples) ? signal[idx] * window_[i] : 0.0f;
        }

        // Perform FFT
        kiss_fftr(fftCfg, frame.data(), fftOut.data());

        // Convert to std::complex
        spectrogram[t].resize(numBins);
        for (int k = 0; k < numBins; ++k) {
            spectrogram[t][k] = std::complex<float>(fftOut[k].r, fftOut[k].i);
        }
    }

    kiss_fftr_free(fftCfg);

    std::cout << "Stft::forward: Processed " << numFrames << " frames, "
              << numBins << " bins per frame" << std::endl;

    return spectrogram;
}

std::vector<float> Stft::inverse(const std::vector<std::vector<std::complex<float>>>& spectrogram) {
    if (spectrogram.empty()) {
        std::cerr << "Stft::inverse: Empty spectrogram" << std::endl;
        return {};
    }

    const int numFrames = spectrogram.size();
    const int numBins = spectrogram[0].size();

    // Estimate output length
    const int outputLength = fftSize_ + (numFrames - 1) * hopSize_;
    std::vector<float> output(outputLength, 0.0f);
    std::vector<float> windowSum(outputLength, 0.0f);

    // Allocate Kiss FFT configuration for inverse
    kiss_fftr_cfg ifftCfg = kiss_fftr_alloc(fftSize_, 1, nullptr, nullptr);
    if (!ifftCfg) {
        std::cerr << "Stft::inverse: Failed to allocate Kiss IFFT configuration" << std::endl;
        return output;
    }

    std::vector<kiss_fft_cpx> fftIn(numBins);
    std::vector<float> frame(fftSize_);

    for (int t = 0; t < numFrames; ++t) {
        // Convert to Kiss FFT format
        for (int k = 0; k < numBins; ++k) {
            fftIn[k].r = spectrogram[t][k].real();
            fftIn[k].i = spectrogram[t][k].imag();
        }

        // Perform IFFT
        kiss_fftri(ifftCfg, fftIn.data(), frame.data());

        // Overlap-add with window
        const int startIdx = t * hopSize_;
        for (int i = 0; i < fftSize_; ++i) {
            const int idx = startIdx + i;
            if (idx < outputLength) {
                output[idx] += frame[i] * window_[i];
                windowSum[idx] += window_[i] * window_[i];
            }
        }
    }

    kiss_fftr_free(ifftCfg);

    // Normalize by window sum
    for (int i = 0; i < outputLength; ++i) {
        if (windowSum[i] > 1e-8f) {
            output[i] /= windowSum[i];
        }
    }

    std::cout << "Stft::inverse: Reconstructed " << outputLength << " samples from "
              << numFrames << " frames" << std::endl;

    return output;
}

} // namespace img2spec
