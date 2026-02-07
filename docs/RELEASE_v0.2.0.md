# Release v0.2.0

## Sound Preview
- In-app playback before export (Preview / Stop Preview)
- Playback position header (current time / total)
- Playhead (cyan line) on spectrogram image during playback

## Target Duration
- Optional "Set target duration" (0.5–600 s)
- Image is time-resampled so output length matches the specified duration

## App Icon
- Custom app icon (`docs/app-icon.png`); macOS builds `.icns` at build time
- Windows: use `resources/img2spec.ico` (from same PNG) for exe icon

## Documentation
- Japanese README ([README_JP.md](../README_JP.md)); link at top of [README.md](../README.md)
- Docs updated for preview, duration, and troubleshooting

---

**Full changelog**: Sound Preview (QAudioSink, playhead, header), target duration (spectrogram time-resample), app icon (PNG/icns/ico), README JP, and related doc updates.
