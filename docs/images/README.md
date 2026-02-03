# Screenshot Images

This directory contains screenshots and documentation images for the img2spec application.

## Required Screenshots

Please add the following screenshots to this directory:

### 1. `screenshot-main.png`
**Description**: Main application window showing:
- Image preview area (with an example image loaded)
- All parameter controls visible
- Estimated duration display
- Render button

**Suggested capture**: Load a sample image and show the default parameter settings.

---

### 2. `screenshot-frequency-guides.png`
**Description**: Frequency guides visualization showing:
- Image preview in logarithmic mode
- Yellow dashed horizontal lines showing frequency markers
- Frequency labels (50Hz, 100Hz, 200Hz, 500Hz, 1kHz, 2kHz, 5kHz, 10kHz, 15kHz)
- Min Freq and Max Freq parameters visible

**Suggested capture**:
1. Load an image with good frequency content
2. Set "Frequency Scale" to "Logarithmic"
3. Show the frequency guide overlay on the image

---

### 3. `screenshot-progress.png`
**Description**: Progress dialog during rendering showing:
- Progress bar with percentage
- Current operation text (e.g., "Griffin-Lim iteration 32 / 64...")
- Cancel button

**Suggested capture**: Click "Render & Export WAV..." and take a screenshot while Griffin-Lim iterations are running (mid-way through, e.g., iteration 30-40).

---

## Image Specifications

- **Format**: PNG (recommended for UI screenshots)
- **Recommended size**:
  - Width: 800-1200px (for good readability in README)
  - Maintain aspect ratio
- **Quality**: High quality, no compression artifacts
- **Background**: If capturing on macOS, use light mode for consistency

## Adding Screenshots

After capturing the screenshots:

1. Save them to this directory with the exact filenames:
   - `screenshot-main.png`
   - `screenshot-frequency-guides.png`
   - `screenshot-progress.png`

2. The README.md in the root directory already references these images:
   ```markdown
   ![Main Window](docs/images/screenshot-main.png)
   ![Frequency Guides](docs/images/screenshot-frequency-guides.png)
   ![Progress Dialog](docs/images/screenshot-progress.png)
   ```

3. Verify they display correctly by viewing the README on GitHub or in a Markdown viewer.

---

## Optional: Additional Documentation Images

You may also add:
- `example-input.png`: Sample input image
- `screenshot-linear-mode.png`: Linear frequency scale comparison
- `screenshot-stereo-options.png`: Stereo output settings
- Any other helpful visual documentation
