#include "core/Leveling.h"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace img2spec {

Leveling::Leveling() {}
Leveling::~Leveling() {}

void Leveling::removeDCOffset(std::vector<float>& audio) {
    if (audio.empty()) return;

    // Calculate mean
    const float mean = std::accumulate(audio.begin(), audio.end(), 0.0f) / audio.size();

    // Subtract mean
    for (float& sample : audio) {
        sample -= mean;
    }
}

void Leveling::normalize(std::vector<float>& audio, double targetDbfs) {
    if (audio.empty()) return;

    // Find peak
    float peak = 0.0f;
    for (const float sample : audio) {
        peak = std::max(peak, std::abs(sample));
    }

    if (peak < 1e-8f) return; // Avoid division by zero

    // Calculate target linear amplitude
    const float targetLinear = std::pow(10.0f, static_cast<float>(targetDbfs) / 20.0f);

    // Calculate scale factor
    const float scale = targetLinear / peak;

    // Apply scale
    for (float& sample : audio) {
        sample *= scale;
    }
}

void Leveling::applyGain(std::vector<float>& audio, double gainDb) {
    if (audio.empty()) return;

    const float gain = std::pow(10.0f, static_cast<float>(gainDb) / 20.0f);

    for (float& sample : audio) {
        sample *= gain;
    }
}

float Leveling::softClip(float sample, float threshold) {
    if (std::abs(sample) <= threshold) {
        return sample;
    }

    // Soft clipping using tanh
    const float sign = (sample >= 0) ? 1.0f : -1.0f;
    const float excess = std::abs(sample) - threshold;
    const float clipped = threshold + (1.0f - threshold) * std::tanh(excess / (1.0f - threshold));

    return sign * clipped;
}

void Leveling::applySafetyLimiter(std::vector<float>& audio, float threshold) {
    for (float& sample : audio) {
        sample = softClip(sample, threshold);
    }
}

std::vector<float> Leveling::monoToStereo(const std::vector<float>& mono) {
    std::vector<float> stereo(mono.size() * 2);

    for (size_t i = 0; i < mono.size(); ++i) {
        stereo[i * 2 + 0] = mono[i]; // L
        stereo[i * 2 + 1] = mono[i]; // R
    }

    return stereo;
}

} // namespace img2spec
