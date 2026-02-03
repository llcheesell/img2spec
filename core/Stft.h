#pragma once

#include <vector>
#include <complex>

namespace img2spec {

class Stft {
public:
    Stft(int fftSize, int hopSize);
    ~Stft();

    // Forward STFT: time-domain signal -> complex spectrogram
    std::vector<std::vector<std::complex<float>>> forward(const std::vector<float>& signal);

    // Inverse STFT: complex spectrogram -> time-domain signal
    std::vector<float> inverse(const std::vector<std::vector<std::complex<float>>>& spectrogram);

    int getFftSize() const { return fftSize_; }
    int getHopSize() const { return hopSize_; }
    int getNumBins() const { return fftSize_ / 2 + 1; }

private:
    void createWindow();

    int fftSize_;
    int hopSize_;
    std::vector<float> window_;
};

} // namespace img2spec
