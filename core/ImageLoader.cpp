#include "core/ImageLoader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cmath>
#include <algorithm>
#include <iostream>

namespace img2spec {

ImageLoader::ImageLoader() {}

ImageLoader::~ImageLoader() {}

bool ImageLoader::load(const std::string& path) {
    int channels = 0;

    // Load image with stb_image
    unsigned char* data = stbi_load(path.c_str(), &width_, &height_, &channels, 0);

    if (!data) {
        std::cerr << "ImageLoader: Failed to load image: " << path << std::endl;
        std::cerr << "  stbi error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "ImageLoader: Loaded image " << path << std::endl;
    std::cout << "  Size: " << width_ << "x" << height_ << std::endl;
    std::cout << "  Channels: " << channels << std::endl;

    // Convert to grayscale
    convertToGrayscale(data, channels);

    // Free stb_image data
    stbi_image_free(data);

    std::cout << "  Converted to grayscale: " << grayscaleData_.size() << " pixels" << std::endl;

    return true;
}

void ImageLoader::convertToGrayscale(const unsigned char* data, int channels) {
    const int numPixels = width_ * height_;
    grayscaleData_.resize(numPixels);

    for (int i = 0; i < numPixels; ++i) {
        const int idx = i * channels;

        if (channels == 1) {
            // Already grayscale
            grayscaleData_[i] = data[idx] / 255.0f;
        } else if (channels == 2) {
            // Grayscale + Alpha (ignore alpha)
            grayscaleData_[i] = data[idx] / 255.0f;
        } else if (channels >= 3) {
            // RGB or RGBA (ignore alpha if present)
            const float r = data[idx + 0] / 255.0f;
            const float g = data[idx + 1] / 255.0f;
            const float b = data[idx + 2] / 255.0f;

            // ITU-R BT.709 luminance formula
            const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            grayscaleData_[i] = luminance;
        }
    }
}

float ImageLoader::getPixel(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return 0.0f;
    }
    return grayscaleData_[y * width_ + x];
}

float ImageLoader::bilinearSample(float x, float y) const {
    // Clamp to valid range
    x = std::max(0.0f, std::min(x, static_cast<float>(width_ - 1)));
    y = std::max(0.0f, std::min(y, static_cast<float>(height_ - 1)));

    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = std::min(x0 + 1, width_ - 1);
    const int y1 = std::min(y0 + 1, height_ - 1);

    const float fx = x - x0;
    const float fy = y - y0;

    const float v00 = getPixel(x0, y0);
    const float v10 = getPixel(x1, y0);
    const float v01 = getPixel(x0, y1);
    const float v11 = getPixel(x1, y1);

    const float v0 = v00 * (1.0f - fx) + v10 * fx;
    const float v1 = v01 * (1.0f - fx) + v11 * fx;

    return v0 * (1.0f - fy) + v1 * fy;
}

std::vector<float> ImageLoader::resample(int newWidth, int newHeight) const {
    if (!isLoaded()) {
        return {};
    }

    std::vector<float> resampled(newWidth * newHeight);

    const float xScale = static_cast<float>(width_) / newWidth;
    const float yScale = static_cast<float>(height_) / newHeight;

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            const float srcX = x * xScale;
            const float srcY = y * yScale;
            resampled[y * newWidth + x] = bilinearSample(srcX, srcY);
        }
    }

    return resampled;
}

} // namespace img2spec
