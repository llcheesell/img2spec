#include "core/WavWriter.h"
#include <sndfile.h>
#include <iostream>

namespace img2spec {

WavWriter::WavWriter() {}
WavWriter::~WavWriter() {}

bool WavWriter::write(
    const std::string& path,
    const std::vector<float>& audio,
    int channels,
    int sampleRate,
    BitDepth bitDepth
) {
    return writeWithLibsndfile(path, audio, channels, sampleRate, bitDepth);
}

bool WavWriter::writeWithLibsndfile(
    const std::string& path,
    const std::vector<float>& audio,
    int channels,
    int sampleRate,
    BitDepth bitDepth
) {
    SF_INFO sfInfo;
    sfInfo.samplerate = sampleRate;
    sfInfo.channels = channels;
    sfInfo.format = SF_FORMAT_WAV;

    // Set subformat based on bit depth
    switch (bitDepth) {
        case BitDepth::Int16:
            sfInfo.format |= SF_FORMAT_PCM_16;
            break;
        case BitDepth::Int24:
            sfInfo.format |= SF_FORMAT_PCM_24;
            break;
        case BitDepth::Float32:
            sfInfo.format |= SF_FORMAT_FLOAT;
            break;
    }

    // Open file for writing
    SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &sfInfo);
    if (!file) {
        std::cerr << "WavWriter: Failed to open file: " << path << std::endl;
        std::cerr << "  libsndfile error: " << sf_strerror(nullptr) << std::endl;
        return false;
    }

    // Write audio data
    const sf_count_t numFrames = audio.size() / channels;
    const sf_count_t written = sf_writef_float(file, audio.data(), numFrames);

    if (written != numFrames) {
        std::cerr << "WavWriter: Write error. Expected " << numFrames
                  << " frames, wrote " << written << std::endl;
        sf_close(file);
        return false;
    }

    // Close file
    sf_close(file);

    std::cout << "WavWriter: Successfully wrote " << path << std::endl;
    std::cout << "  Frames: " << numFrames << std::endl;
    std::cout << "  Channels: " << channels << std::endl;
    std::cout << "  Sample Rate: " << sampleRate << " Hz" << std::endl;
    std::cout << "  Bit Depth: ";
    switch (bitDepth) {
        case BitDepth::Int16: std::cout << "16-bit PCM"; break;
        case BitDepth::Int24: std::cout << "24-bit PCM"; break;
        case BitDepth::Float32: std::cout << "32-bit Float"; break;
    }
    std::cout << std::endl;

    return true;
}

} // namespace img2spec
