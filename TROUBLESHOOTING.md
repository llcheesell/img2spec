# img2spec - Troubleshooting Guide

## Common Issues and Solutions

### Issue: File Dialog Doesn't Appear

**Symptoms:**
- Clicking "Open Image..." button doesn't show a file selection dialog
- Application appears to freeze or not respond

**Solution:**
This was fixed in commit `4e9535d` by adding `QFileDialog::DontUseNativeDialog` flag.

**Technical Details:**
- macOS native file dialog can sometimes appear behind other windows
- Qt6's native dialog integration may have timing issues
- The fix forces Qt to use its own cross-platform dialog instead

**Verification:**
```bash
# Check if the fix is applied
grep "DontUseNativeDialog" app/MainWindow.cpp
```

### Issue: Drag and Drop Not Working

**Symptoms:**
- Dropping image files on the window doesn't load them
- No visual feedback when dragging files over the window

**Checklist:**
1. ✅ `setAcceptDrops(true)` is called in constructor
2. ✅ `dragEnterEvent()` and `dropEvent()` are implemented
3. ✅ File extensions are checked (.png, .jpg, .jpeg)

**Debug:**
```bash
# Run app and check console output
./build/img2spec.app/Contents/MacOS/img2spec 2>&1 | grep -i "drag"
```

Expected output when working:
```
Drag and drop enabled
Drag enter event detected
Accepting drag: /path/to/image.png
Drop event detected
Dropped file: /path/to/image.png
```

### Issue: Image Loads But Preview Doesn't Show

**Symptoms:**
- Console shows "Image loaded successfully"
- Preview area remains gray/empty

**Possible Causes:**
1. QImage format mismatch
2. Pixmap scaling issue
3. Label not updating

**Solution:**
Check the console for:
```
Preview updated: 769x769
```

If this line appears, the preview is being updated. The image might be:
- Too large and scaled down
- In an unsupported format
- Corrupted

**Verification:**
```bash
# Test with a known-good image
open /tmp/test_spectrogram.png  # Should open in Preview.app
```

### Issue: Application Crashes on Startup

**Symptoms:**
- App launches but immediately crashes
- No GUI window appears

**Common Causes:**
1. Missing Qt6 libraries
2. AGL framework not found (macOS)
3. Missing dependencies

**Solution:**

Check library paths:
```bash
otool -L build/img2spec.app/Contents/MacOS/img2spec | grep Qt
```

Check AGL framework:
```bash
ls -la build/img2spec.app/Contents/Frameworks/AGL.framework/
```

If AGL is missing, rebuild:
```bash
rm -rf build
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/qt@6
cmake --build . --config Release
```

### Issue: Render Button Doesn't Work

**Symptoms:**
- Clicking "Render & Export WAV..." does nothing
- No file save dialog appears

**Debug Steps:**

1. Check if image is loaded:
```bash
# Console should show
Image loaded successfully: 769x769
```

2. Check if button is enabled:
- Button should be clickable after image loads
- If grayed out, image didn't load properly

3. Run with verbose output:
```bash
./build/img2spec.app/Contents/MacOS/img2spec 2>&1 | tee render_debug.log
# Click render and check log
```

### Issue: WAV File Generation Fails

**Symptoms:**
- Render starts but fails with error
- Empty or corrupt WAV file

**Common Causes:**
1. Invalid output path
2. Disk space issues
3. Permission errors
4. libsndfile not working

**Solution:**

Check disk space:
```bash
df -h /path/to/output
```

Test libsndfile:
```bash
# Check if libsndfile is linked
otool -L build/img2spec.app/Contents/MacOS/img2spec | grep sndfile
```

Verify output path:
- Use a simple path like `~/Desktop/test.wav`
- Avoid special characters in filename

### Issue: Qt6 Keyboard Warnings in Console

**Symptoms:**
```
qt.qpa.keymapper: Mismatch between Cocoa ' ' and Carbon '\x0' for virtual key 49
```

**Status:** 
⚠️ **HARMLESS** - These are Qt6 warnings about keyboard mapping on macOS. They don't affect functionality.

**Why it happens:**
- Qt6 uses modern Cocoa APIs
- Legacy Carbon key mapping is deprecated
- Mismatch is expected and can be ignored

**Suppress (optional):**
```bash
export QT_LOGGING_RULES="qt.qpa.keymapper=false"
./build/img2spec.app/Contents/MacOS/img2spec
```

### Issue: Build Fails with AGL Framework Error

**Symptoms:**
```
ld: framework 'AGL' not found
```

**Solution:**
The build system should automatically create a dummy AGL framework. If it fails:

```bash
# Manual fix
cd build
mkdir -p AGL.framework/Versions/A
cd AGL.framework
ln -sf Versions/A/AGL AGL
cd Versions
ln -sf A Current
cd A
echo "void _dummy_agl(void) {}" > empty.c
cc -dynamiclib -o AGL empty.c
cd ../../..

# Rebuild
cmake --build . --config Release
```

### Issue: Preview Playback Has No Sound or Fails

**Symptoms:**
- Click "Preview" but no audio, or an error dialog
- Playback header shows "Preview: 0:00.0 / X.X" but no sound

**Possible causes and fixes:**
1. **No audio output device**: Ensure the system has a default playback device (speakers/headphones). On macOS, check Sound settings.
2. **Format not supported**: The app tries Float then Int16. If you see "Audio device does not support the current format", try a different sample rate (e.g. 44100 Hz) or disable "Stereo" for preview.
3. **Empty or very quiet audio**: Same as exported WAV — check image brightness, Min dB, and gamma (see "Image Loads But Audio Is Silent/Corrupt" below).
4. **Playhead not moving**: If the playhead (cyan line) does not move, playback may have ended or failed; check the playback header for time progress.

### Issue: Image Loads But Audio Is Silent/Corrupt

**Symptoms:**
- WAV file is generated
- File size looks correct
- No sound when playing (exported file or in-app Preview)

**Debug Steps:**

1. Check image brightness:
   - Very dark images → very quiet audio
   - All black → silent
   - Try increasing gamma (e.g., 2.0)

2. Check parameters:
   - MinDB too low (e.g., -120) → too quiet
   - Try -60 dB for testing

3. Verify WAV file:
```bash
ffprobe output.wav
# Should show valid audio stream
```

4. Check for clipping:
   - Enable Safety Limiter
   - Reduce Output Gain if distorted

### Performance Issues

**Symptoms:**
- Render takes very long time
- Application becomes unresponsive
- High CPU usage

**Expected Performance:**
- 256x128 image, 64 iterations: ~2-3 seconds
- 1024x512 image, 64 iterations: ~15-30 seconds

**Optimization Tips:**
1. Reduce Griffin-Lim iterations (try 32)
2. Use smaller FFT size (1024 instead of 4096)
3. Resize large images before loading
4. Close other applications

**Technical Limits:**
- Max recommended: 4096x4096 pixels
- Memory usage: ~100MB per 1000x1000 image
- Processing scales with: width × iterations × FFT_size

### Getting Help

**Before Reporting Issues:**

1. Check console output for errors
2. Verify image file is valid (opens in other apps)
3. Try with test image: `/tmp/test_spectrogram.png`
4. Test with minimal parameters (small FFT, few iterations)

**Include in Bug Report:**
- macOS version: `sw_vers`
- Qt6 version: `qmake --version`
- Build log: `cmake --build . 2>&1 | tee build.log`
- Runtime log: `./img2spec 2>&1 | tee runtime.log`
- Image size and format
- Parameters used

**Useful Debug Commands:**
```bash
# Check all dependencies
otool -L build/img2spec.app/Contents/MacOS/img2spec

# Monitor resource usage
top -pid $(pgrep img2spec)

# Trace system calls (advanced)
sudo dtruss -p $(pgrep img2spec) 2>&1 | grep open
```

## Known Limitations

1. **UI Freeze During Render**: Normal, not a bug. Progress bar updates but window may appear frozen.
2. **No Cancel**: Cancel button not yet implemented
3. **Single-threaded**: Render happens on main thread
4. **Memory**: Large images (>4096) may cause slowdowns

## Version Information

- Last Updated: 2026-02-03
- Version: 1.0.0
- Commit: 4e9535d

