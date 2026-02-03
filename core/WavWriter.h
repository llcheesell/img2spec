#pragma once

#include <string>
#include <vector>

namespace img2spec {

enum class BitDepth {
    Int16,
    Int24,
    Float32
};

class WavWriter {
public:
    WavWriter();
    ~WavWriter();

    // Write audio to WAV file
    // audio: interleaved samples (mono: [s0, s1, ...], stereo: [L0, R0, L1, R1, ...])
    // channels: 1 (mono) or 2 (stereo)
    // sampleRate: sample rate in Hz
    // bitDepth: bit depth format
    // Returns: true if successful
    bool write(
        const std::string& path,
        const std::vector<float>& audio,
        int channels,
        int sampleRate,
        BitDepth bitDepth
    );

private:
    bool writeWithLibsndfile(
        const std::string& path,
        const std::vector<float>& audio,
        int channels,
        int sampleRate,
        BitDepth bitDepth
    );
};

} // namespace img2spec
