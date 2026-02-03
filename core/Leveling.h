#pragma once

#include <vector>

namespace img2spec {

class Leveling {
public:
    Leveling();
    ~Leveling();

    // Remove DC offset
    static void removeDCOffset(std::vector<float>& audio);

    // Normalize to target dBFS
    static void normalize(std::vector<float>& audio, double targetDbfs);

    // Apply gain in dB
    static void applyGain(std::vector<float>& audio, double gainDb);

    // Apply soft limiter to prevent clipping
    static void applySafetyLimiter(std::vector<float>& audio, float threshold = 0.99f);

    // Convert mono to stereo (duplicate L/R)
    static std::vector<float> monoToStereo(const std::vector<float>& mono);

private:
    static float softClip(float sample, float threshold);
};

} // namespace img2spec
