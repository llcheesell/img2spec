# Quick Release Guide

**TL;DR**: How to create a GitHub Release in 3 steps.

## Prerequisites

- All changes committed
- Qt6 installed: `brew install qt@6`

## Steps

### 1. Build & Package

```bash
# Build in Release mode
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
cmake --build . --config Release
cd ..

# Create distribution package
./scripts/create-macos-release.sh v0.1.0

# Test the release
open release/img2spec.app
```

### 2. Create Git Tag

```bash
git tag -a v0.1.0 -m "Release v0.1.0: Description"
git push origin v0.1.0
```

### 3. Create GitHub Release

Go to: https://github.com/YOUR_USERNAME/img2spec/releases/new

- **Tag**: Select `v0.1.0`
- **Title**: `img2spec v0.1.0`
- **Description**: Write release notes
- **Attach files**:
  - `release/img2spec-v0.1.0-macos.dmg` ⭐ (recommended)
  - `release/img2spec-v0.1.0-macos.zip` (alternative)
- Click **"Publish release"**

Done! 🎉

---

For detailed instructions, see [RELEASE.md](RELEASE.md)
