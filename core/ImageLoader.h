#pragma once

#include <vector>
#include <string>
#include <memory>

namespace img2spec {

/**
 * ImageLoader: Loads PNG/JPG images and converts to grayscale
 * - Handles RGBA/RGB images
 * - Converts to grayscale using luminance formula
 * - Ignores alpha channel
 * - Supports bilinear resampling
 */
class ImageLoader {
public:
    ImageLoader();
    ~ImageLoader();

    /**
     * Load image from file path
     * @param path File path to PNG or JPG
     * @return true if successful
     */
    bool load(const std::string& path);

    /**
     * Get grayscale data (normalized to [0.0, 1.0])
     * @return Pointer to grayscale data (row-major, top to bottom)
     */
    const std::vector<float>& getGrayscaleData() const { return grayscaleData_; }

    /**
     * Get image dimensions
     */
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    /**
     * Resample image to new dimensions using bilinear interpolation
     * @param newWidth Target width
     * @param newHeight Target height
     * @return Resampled grayscale data
     */
    std::vector<float> resample(int newWidth, int newHeight) const;

    /**
     * Get grayscale value at position (with bounds checking)
     * @param x Horizontal position (0..width-1)
     * @param y Vertical position (0..height-1)
     * @return Grayscale value [0.0, 1.0]
     */
    float getPixel(int x, int y) const;

    /**
     * Check if image is loaded
     */
    bool isLoaded() const { return !grayscaleData_.empty(); }

private:
    void convertToGrayscale(const unsigned char* data, int channels);
    float bilinearSample(float x, float y) const;

    int width_ = 0;
    int height_ = 0;
    std::vector<float> grayscaleData_; // Normalized [0.0, 1.0]
};

} // namespace img2spec
