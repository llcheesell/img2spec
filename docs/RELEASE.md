# Release Process Guide

This document describes how to create and publish a new release of img2spec.

## Prerequisites

- macOS with Xcode Command Line Tools installed
- Qt6 installed via Homebrew: `brew install qt@6`
- Git repository with all changes committed
- Clean working directory: `git status` shows no uncommitted changes

## Release Checklist

### 1. Pre-Release Preparation

- [ ] All features tested and working
- [ ] Documentation updated (README.md, IMPLEMENTATION_SUMMARY.md)
- [ ] Screenshots added to `docs/images/`
- [ ] All changes committed to git
- [ ] Version number decided (e.g., v0.1.0)

### 2. Create Release Build

```bash
# Clean previous build
rm -rf build
mkdir build
cd build

# Configure in Release mode
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6

# Build
cmake --build . --config Release

# Test the built app
open img2spec.app
```

**Important**: Test the release build thoroughly before proceeding!

### 3. Create macOS Distribution Package

Run the automated release script:

```bash
# From project root directory
./scripts/create-macos-release.sh v0.1.0
```

This script will:
1. Copy the app bundle to `release/` directory
2. Run `macdeployqt` to bundle Qt frameworks
3. Create a `.dmg` disk image for distribution
4. Create a `.zip` archive (alternative format)

**Output files**:
- `release/img2spec.app` - Standalone app bundle
- `release/img2spec-v0.1.0-macos.dmg` - DMG installer (recommended for users)
- `release/img2spec-v0.1.0-macos.zip` - ZIP archive (alternative)

### 4. Test Distribution Package

```bash
# Test the app from release directory
open release/img2spec.app

# Test the DMG
open release/img2spec-v0.1.0-macos.dmg
# Drag app to Applications folder and test
```

**Verify**:
- App launches without errors
- All features work correctly
- No missing Qt libraries (check Console.app for errors)
- Image loading, rendering, and WAV export all functional

### 5. Create Git Tag

```bash
# Create annotated tag
git tag -a v0.1.0 -m "Release v0.1.0: Initial release with frequency guides and duration estimation"

# Verify tag
git tag -l
git show v0.1.0

# Push tag to GitHub
git push origin v0.1.0
```

### 6. Create GitHub Release

#### Option A: Via GitHub Web Interface (Recommended)

1. Go to: https://github.com/YOUR_USERNAME/img2spec/releases/new

2. **Choose a tag**: Select `v0.1.0` from dropdown (or the tag you just pushed)

3. **Release title**: `img2spec v0.1.0`

4. **Describe this release**: Write release notes, for example:

```markdown
## img2spec v0.1.0 - Initial Release

Image to Spectrogram Audio Generator - Convert images to audio using the Griffin-Lim algorithm.

### ✨ Features

- 🖼️ **Image Loading**: PNG/JPG support with drag & drop
- 🎵 **Audio Generation**: Griffin-Lim phase reconstruction with STFT/ISTFT
- 📊 **Frequency Visualization**: Visual guides for logarithmic frequency mapping
- ⏱️ **Duration Estimation**: Real-time output duration calculation
- 🎛️ **Full Parameter Control**: Sample rate, bit depth, FFT size, frequency range, and more
- 💾 **WAV Export**: 16/24/32-bit PCM and Float formats

### 📦 Installation (macOS)

**Option 1: DMG Installer (Recommended)**
1. Download `img2spec-v0.1.0-macos.dmg`
2. Open the DMG file
3. Drag `img2spec.app` to your Applications folder
4. Launch from Applications

**Option 2: ZIP Archive**
1. Download `img2spec-v0.1.0-macos.zip`
2. Extract the ZIP file
3. Move `img2spec.app` to your Applications folder
4. Right-click and select "Open" (first time only, due to macOS Gatekeeper)

### 🛠️ Technical Details

- **GUI**: Qt6
- **DSP**: kissfft, Griffin-Lim algorithm
- **Audio I/O**: libsndfile
- **Supported Formats**: PNG, JPG input / WAV output
- **Platforms**: macOS (tested on macOS 14+)

### 📖 Documentation

See [README.md](https://github.com/YOUR_USERNAME/img2spec#readme) for full documentation.

### 🐛 Known Issues

- Rendering happens on main thread (UI may freeze briefly during render)
- Cancel button in progress dialog is not functional yet

### 🙏 Acknowledgments

Built with Claude Code - AI-powered development assistant.
```

5. **Attach binaries**:
   - Drag and drop `release/img2spec-v0.1.0-macos.dmg`
   - Drag and drop `release/img2spec-v0.1.0-macos.zip`

6. **Set as latest release**: Check "Set as the latest release"

7. Click **"Publish release"**

#### Option B: Via GitHub CLI (gh)

```bash
# Install GitHub CLI if needed
brew install gh

# Authenticate
gh auth login

# Create release with binaries
gh release create v0.1.0 \
  release/img2spec-v0.1.0-macos.dmg \
  release/img2spec-v0.1.0-macos.zip \
  --title "img2spec v0.1.0" \
  --notes "Initial release with frequency guides and duration estimation. See README for full documentation."
```

### 7. Post-Release

- [ ] Verify release appears on GitHub Releases page
- [ ] Download and test the release binaries
- [ ] Update README.md with download link if needed
- [ ] Announce release (social media, forums, etc.)

## Version Numbering

We use Semantic Versioning (semver): `MAJOR.MINOR.PATCH`

- **MAJOR**: Breaking changes, major feature overhauls
- **MINOR**: New features, non-breaking changes
- **PATCH**: Bug fixes, minor improvements

Examples:
- `v0.1.0` - Initial release
- `v0.1.1` - Bug fix release
- `v0.2.0` - New features added
- `v1.0.0` - First stable release

## Troubleshooting

### macdeployqt not found

```bash
# Install Qt6 via Homebrew
brew install qt@6

# Add to PATH
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
```

### App doesn't launch after distribution

Check for missing frameworks:
```bash
# Check what's bundled
ls -la release/img2spec.app/Contents/Frameworks/

# Re-run macdeployqt manually
/opt/homebrew/opt/qt@6/bin/macdeployqt release/img2spec.app -verbose=1
```

### "App is damaged and can't be opened" error

This happens when downloading from the internet. Users should:
```bash
xattr -cr /path/to/img2spec.app
```

Or right-click → Open (bypasses Gatekeeper on first launch).

**For future releases**: Consider code signing the app with an Apple Developer certificate.

## Automated Release (Future)

For future releases, consider setting up GitHub Actions to automate:
- Building on multiple platforms (macOS, Windows)
- Running tests
- Creating release artifacts
- Publishing to GitHub Releases

Example workflow: `.github/workflows/release.yml`

---

**Last Updated**: 2025-02-03
**Version**: 1.0
