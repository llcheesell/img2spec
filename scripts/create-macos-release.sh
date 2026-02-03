#!/bin/bash
# Create macOS release package for img2spec
# Usage: ./scripts/create-macos-release.sh [version]

set -e

VERSION=${1:-"v0.1.0"}
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build"
RELEASE_DIR="$PROJECT_DIR/release"
APP_NAME="img2spec"

echo "=== Creating macOS Release Package ==="
echo "Version: $VERSION"
echo "Project: $PROJECT_DIR"
echo ""

# Clean and create release directory
rm -rf "$RELEASE_DIR"
mkdir -p "$RELEASE_DIR"

# Check if build exists
if [ ! -d "$BUILD_DIR/$APP_NAME.app" ]; then
    echo "ERROR: $BUILD_DIR/$APP_NAME.app not found!"
    echo "Please run: cmake --build build --config Release"
    exit 1
fi

echo "Step 1: Copying app bundle..."
cp -R "$BUILD_DIR/$APP_NAME.app" "$RELEASE_DIR/"

echo "Step 2: Running macdeployqt to bundle Qt frameworks..."
# Find Qt installation
QT_DIR=$(brew --prefix qt@6 2>/dev/null || echo "/opt/homebrew/opt/qt@6")
MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

if [ ! -f "$MACDEPLOYQT" ]; then
    echo "WARNING: macdeployqt not found at $MACDEPLOYQT"
    echo "Trying to find it in PATH..."
    MACDEPLOYQT=$(which macdeployqt || echo "")
    if [ -z "$MACDEPLOYQT" ]; then
        echo "ERROR: macdeployqt not found. Install Qt6 via Homebrew:"
        echo "  brew install qt@6"
        exit 1
    fi
fi

echo "Using macdeployqt: $MACDEPLOYQT"
"$MACDEPLOYQT" "$RELEASE_DIR/$APP_NAME.app" -verbose=1

echo ""
echo "Step 3: Creating DMG (disk image)..."
DMG_NAME="$APP_NAME-$VERSION-macos.dmg"
DMG_PATH="$RELEASE_DIR/$DMG_NAME"

# Remove old DMG if exists
rm -f "$DMG_PATH"

# Create DMG
hdiutil create -volname "$APP_NAME $VERSION" \
    -srcfolder "$RELEASE_DIR/$APP_NAME.app" \
    -ov -format UDZO \
    "$DMG_PATH"

echo ""
echo "Step 4: Creating ZIP archive..."
ZIP_NAME="$APP_NAME-$VERSION-macos.zip"
ZIP_PATH="$RELEASE_DIR/$ZIP_NAME"

cd "$RELEASE_DIR"
zip -r -q "$ZIP_NAME" "$APP_NAME.app"
cd "$PROJECT_DIR"

echo ""
echo "=== Release Package Created Successfully ==="
echo ""
echo "Output files:"
echo "  - App Bundle: $RELEASE_DIR/$APP_NAME.app"
echo "  - DMG Image:  $DMG_PATH"
echo "  - ZIP Archive: $ZIP_PATH"
echo ""
echo "File sizes:"
ls -lh "$DMG_PATH" | awk '{print "  DMG: " $5}'
ls -lh "$ZIP_PATH" | awk '{print "  ZIP: " $5}'
echo ""
echo "Next steps:"
echo "1. Test the app bundle in release/ directory"
echo "2. Create a git tag: git tag -a $VERSION -m \"Release $VERSION\""
echo "3. Push tag: git push origin $VERSION"
echo "4. Create GitHub Release and upload:"
echo "   - $DMG_NAME (for macOS users)"
echo "   - $ZIP_NAME (alternative format)"
echo ""
