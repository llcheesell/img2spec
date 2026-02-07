# img2spec - Image to Spectrogram Audio Generator

**日本語** → [README_JP.md](README_JP.md)

Convert images (PNG/JPG) into audio by interpreting them as spectrograms. Uses the Griffin-Lim algorithm for phase reconstruction and ISTFT for high-quality audio synthesis.

## Screenshots

### Application Interface
![Main Window](docs/images/screenshot-main.png)
*Main application window with image preview, frequency guides, and comprehensive parameter controls*

### Output Result
![Spectrogram Result](docs/images/spectrogram-result.png)
*Generated audio visualized as spectrogram in Adobe Audition - showing accurate frequency reconstruction*

## Features

- **Cross-platform**: Windows and macOS support
- **Image formats**: PNG, JPG (auto-converts color to grayscale)
- **Audio output**: WAV format with multiple options:
  - Sample rates: 44.1kHz, 48kHz, 96kHz
  - Bit depths: 16-bit PCM, 24-bit PCM, 32-bit Float
  - Mono or Stereo (L/R duplicate)
- **DSP controls**:
  - FFT size: 1024, 2048, 4096
  - Hop size: NFFT/2, NFFT/4, NFFT/8
  - Frequency scale: Linear, Logarithmic (20Hz - 20kHz configurable)
  - Brightness mapping: minDb, gamma correction
  - Griffin-Lim iterations (16-256)
  - Normalize, Output Gain, Safety Limiter
- **Enhanced UX**:
  - Visual frequency guides on image preview (logarithmic mode)
  - Real-time audio duration estimation; optional **target duration** (time-resample to user-defined length)
  - **In-app Sound Preview**: play before export with position header (current / total) and playhead on the spectrogram image; stop with "Stop Preview"
  - Drag & drop support for images
  - Detailed progress dialog during rendering

## Installation

### Download Source Code

Download the latest release from the [Releases page](https://github.com/YOUR_USERNAME/img2spec/releases):
- Click on "Source code (zip)" or "Source code (tar.gz)"
- Extract the archive
- Follow the build instructions below

Alternatively, clone the repository:
```bash
git clone https://github.com/YOUR_USERNAME/img2spec.git
cd img2spec
```

**Note**: Pre-built binaries are not available due to macOS code-signing requirements. Building from source ensures compatibility with your system.

## Technology Stack

- **GUI**: Qt6 (cross-platform UI)
- **FFT**: kissfft (lightweight, BSD license)
- **Audio I/O**: libsndfile (supports all PCM formats)
- **Image loading**: stb_image (PNG/JPG support)
- **Build**: CMake 3.20+

## Build from Source

### macOS Requirements

```bash
# Install Qt6
brew install qt6

# Install libsndfile (optional, will be fetched if not found)
brew install libsndfile

# Install CMake (if needed)
brew install cmake
```

### Windows Requirements

1. Install Qt6 from https://www.qt.io/download
2. Install CMake from https://cmake.org/download/
3. Install Visual Studio 2019 or later with C++ support

#### macOS Build Steps

```bash
# Clone the repository
cd /path/to/img2spec

# Create build directory
mkdir build
cd build

# Configure (specify Qt6 path if needed)
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

# Build
cmake --build . --config Release

# Run
./img2spec
```

#### Windows Build Steps (Visual Studio)

```bash
# Open Developer Command Prompt
cd C:\path\to\img2spec

# Create build directory
mkdir build
cd build

# Configure (specify Qt6 path)
cmake .. -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2019_64

# Build
cmake --build . --config Release

# Run
Release\img2spec.exe
```

## Usage

### Quick Start

1. **Launch the application**:
   ```bash
   ./build/img2spec.app/Contents/MacOS/img2spec
   ```

2. **Open Image**:
   - **Method 1**: Click "Open Image..." and select a PNG or JPG file
   - **Method 2**: Drag and drop an image file onto the application window
   - Color images are automatically converted to grayscale
   - Alpha channel is ignored
   - Preview appears in the window

3. **Review Audio Duration**:
   - The estimated output duration is displayed below the parameters
   - **From image**: `Duration = (Image Width × Hop Size) / Sample Rate`
   - **Optional**: Check "Set target duration" and enter seconds (0.5–600); the image is time-resampled so the output length matches

4. **Adjust Parameters**: Configure settings for your desired output
   - Start with defaults for first test
   - Recommended settings for good quality:
     - Sample Rate: 48000 Hz
     - Bit Depth: 24 bit (PCM)
     - FFT Size: 2048
     - Hop Size: NFFT/4
     - Frequency Scale: Logarithmic
     - Min Freq: 20 Hz, Max Freq: 20000 Hz
     - Griffin-Lim Iterations: 64

5. **Frequency Guides** (Logarithmic mode only):
   - Visual guides show frequency positions on the image
   - Common frequencies marked: 50Hz, 100Hz, 200Hz, 500Hz, 1kHz, 2kHz, 5kHz, 10kHz, 15kHz
   - Helps understand which parts of the image correspond to which frequencies

6. **Sound Preview**: Click "Preview" to audition audio in-app
   - Playback uses current settings without exporting a file
   - A **playback header** above the image shows current time / total (e.g. `Preview: 0:02.3 / 0:05.1`)
   - A **playhead** (cyan vertical line) moves across the spectrogram image during playback
   - Click "Stop Preview" to stop

7. **Render**: Click "Render & Export WAV..."
   - Choose save location for WAV file
   - Progress dialog shows detailed rendering status:
     - Spectrogram building
     - Griffin-Lim iterations (with count)
     - Post-processing
     - WAV file writing
   - Success dialog displays when complete

8. **Listen**: Open the generated WAV file in any audio player or DAW

### Parameter Guide

- **FFT Size**: Larger = better frequency resolution, longer processing
- **Hop Size**: Smaller = smoother time resolution, longer audio duration
- **Frequency Scale**:
  - **Linear**: Direct pixel-to-bin mapping (uniform frequency distribution)
  - **Logarithmic**: Perceptual mapping (more resolution in low frequencies)
- **Min Freq / Max Freq**: Frequency range for logarithmic mapping (20Hz - 20kHz default)
- **Min dB**: Controls dynamic range (black pixel amplitude)
- **Gamma**: Brightness curve (>1 = brighter, <1 = darker)
- **Griffin-Lim Iterations**: More = better phase estimation (diminishing returns >64)
- **Normalize Target**: Peak level in dBFS (recommended: -1 dBFS)
- **Output Gain**: Additional volume adjustment
- **Safety Limiter**: Prevents clipping with soft limiting
- **Set target duration**: When checked, output length is resampled to the given "Duration (s)" (0.5–600 s)

## Known Limitations

- Maximum recommended image size: 4096x4096 pixels
- Very wide images will have long processing times
- 32-bit output is Float32 format (not Int32)

## License

This project uses:
- Qt6: LGPL v3
- kissfft: BSD
- libsndfile: LGPL v2.1+
- stb_image: Public Domain

## Troubleshooting

### Build Issues

**Qt6 not found**:
```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt6
```

**libsndfile not found**: CMake will automatically fetch it via FetchContent

### Runtime Issues

**"Failed to load image"**: Ensure image is valid PNG/JPG
**"Render failed"**: Check console output for detailed error messages

## Development

### Project Structure

```
img2spec/
├── app/
│   ├── main.cpp                    # Application entry point
│   ├── MainWindow.h/cpp            # Main GUI window
│   ├── ImagePreviewWidget.h/cpp    # Custom preview with frequency guides
├── core/
│   ├── ImageLoader.h/cpp           # Image loading & grayscale conversion
│   ├── SpectrogramBuilder.h/cpp    # Image → magnitude spectrogram
│   ├── Stft.h/cpp                  # STFT/ISTFT implementation
│   ├── GriffinLim.h/cpp            # Griffin-Lim phase reconstruction
│   ├── Leveling.h/cpp              # DC removal, normalize, gain, limiter
│   ├── WavWriter.h/cpp             # WAV file export
├── docs/
│   ├── images/                    # Screenshots and documentation images
│   └── ICON_PROMPT.md              # App icon design and ChatGPT prompt for release
├── CMakeLists.txt                 # Build configuration
└── README.md                      # This file
```

### Build Log Location

Console output includes detailed logging for debugging:
- Image loading details
- Spectrogram generation progress
- Griffin-Lim iteration status
- WAV export confirmation

## Credits

llcheesell, Claude Code, and Cursor — collaborative development.
